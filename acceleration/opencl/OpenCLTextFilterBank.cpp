/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OpenCLTextFilterBank.h"
#include "OpenCLGaussBlur.h"
#include "Utils.h"
#include <QPoint>
#include <QPointF>
#include <utility>
#include <cassert>
#include <cmath>
#include <cstdint>

/** @see AcceleratableOperations::textFilterBank() */
std::pair<OpenCLGrid<float>, OpenCLGrid<uint8_t>> textFilterBank(
	cl::CommandQueue const& command_queue,
	cl::Program const& program,
	OpenCLGrid<float> const& src_grid,
	std::vector<Vec2f> const& directions,
	std::vector<Vec2f> const& sigmas, float const shoulder_length,
	std::vector<cl::Event>* wait_for, cl::Event* event)
{
	cl::Context const context = command_queue.getInfo<CL_QUEUE_CONTEXT>();
	cl::Device const device = command_queue.getInfo<CL_QUEUE_DEVICE>();
	size_t const max_work_group_size = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
	size_t const cacheline_size = device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>();

	size_t const h_wg_size = std::min<size_t>(max_work_group_size, cacheline_size / sizeof(float));
	size_t const v_wg_size = max_work_group_size / h_wg_size;

	std::vector<cl::Event> deps;
	if (wait_for) {
		deps.insert(deps.end(), wait_for->begin(), wait_for->end());
	}

	cl::Event evt;

	cl::Buffer accum_buffer(
		context, CL_MEM_READ_WRITE, src_grid.totalBytesWithDifferentPadding(0)
	);
	OpenCLGrid<float> accum_grid(src_grid.withDifferentPadding(accum_buffer, 0));

	{ // Fill accum_buffer kernel scope.

		cl::Kernel kernel(program, "fill_float_grid");
		int idx = 0;
		kernel.setArg(idx++, accum_grid.width());
		kernel.setArg(idx++, accum_grid.height());
		kernel.setArg(idx++, accum_grid.buffer());
		kernel.setArg(idx++, accum_grid.offset());
		kernel.setArg(idx++, accum_grid.stride());
		kernel.setArg(idx++, cl_float(-std::numeric_limits<float>::max()));

		command_queue.enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(
				thisOrNextMultipleOf(accum_grid.width(), h_wg_size),
				thisOrNextMultipleOf(accum_grid.height(), v_wg_size)
			),
			cl::NDRange(h_wg_size, v_wg_size),
			&deps,
			&evt
		);
		deps.clear();
		deps.push_back(evt);
	}

	cl::Buffer dir_map_buffer(
		context, CL_MEM_READ_WRITE, src_grid.totalBytesWithDifferentPadding<uint8_t>(0)
	);
	OpenCLGrid<uint8_t> dir_map_grid(src_grid.withDifferentPadding<uint8_t>(dir_map_buffer, 0));

	{ // Fill grid kernel scope.

		cl::Kernel kernel(program, "fill_byte_grid");
		int idx = 0;
		kernel.setArg(idx++, dir_map_grid.width());
		kernel.setArg(idx++, dir_map_grid.height());
		kernel.setArg(idx++, dir_map_grid.buffer());
		kernel.setArg(idx++, dir_map_grid.offset());
		kernel.setArg(idx++, dir_map_grid.stride());
		kernel.setArg(idx++, cl_uchar(0));

		command_queue.enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(
				thisOrNextMultipleOf(dir_map_grid.width(), h_wg_size),
				thisOrNextMultipleOf(dir_map_grid.height(), v_wg_size)
			),
			cl::NDRange(h_wg_size, v_wg_size),
			&deps,
			&evt
		);
		deps.clear();
		deps.push_back(evt);
	}

	for (Vec2f const& s : sigmas) {
		for (size_t dir_idx = 0; dir_idx < directions.size(); ++dir_idx) {
			Vec2f const& dir = directions[dir_idx];
			assert(std::abs(dir.squaredNorm() - 1.f) < 1e-5);

			OpenCLGrid<float> blurred_grid = anisotropicGaussBlur(
				command_queue, program, src_grid, dir[0], dir[1], s[0], s[1], &deps, &evt
			);

			QPointF shoulder_f(dir[1], -dir[0]);
			shoulder_f *= s[1] * shoulder_length;
			QPoint const shoulder_i(shoulder_f.toPoint());

			cl::Kernel kernel(program, "text_filter_bank_combine");
			int idx = 0;
			kernel.setArg(idx++, src_grid.width());
			kernel.setArg(idx++, src_grid.height());
			kernel.setArg(idx++, blurred_grid.buffer());
			kernel.setArg(idx++, blurred_grid.offset());
			kernel.setArg(idx++, blurred_grid.stride());
			kernel.setArg(idx++, accum_grid.buffer());
			kernel.setArg(idx++, accum_grid.offset());
			kernel.setArg(idx++, accum_grid.stride());
			kernel.setArg(idx++, dir_map_grid.buffer());
			kernel.setArg(idx++, dir_map_grid.offset());
			kernel.setArg(idx++, dir_map_grid.stride());
			kernel.setArg(idx++, cl_int2{shoulder_i.x(), shoulder_i.y()});
			kernel.setArg(idx++, cl_uchar(dir_idx));

			command_queue.enqueueNDRangeKernel(
				kernel,
				cl::NullRange,
				cl::NDRange(
					thisOrNextMultipleOf(src_grid.width(), h_wg_size),
					thisOrNextMultipleOf(src_grid.height(), v_wg_size)
				),
				cl::NDRange(h_wg_size, v_wg_size),
				&deps,
				&evt
			);
			deps.clear();
			deps.push_back(evt);

			evt.wait(); // Prevent excessive resource consumption.
		}
	}

	if (event) {
		*event = evt;
	}

	return std::make_pair(std::move(accum_grid), std::move(dir_map_grid));
}
