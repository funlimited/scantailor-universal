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

#include "Utils.h"
#include <QString>
#include <QFile>
#include <QDebug>
#include <stdexcept>
#include <utility>

namespace opencl
{

namespace tests
{

DeviceListFixture::DeviceListFixture()
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	for (cl::Platform const& platform : platforms) {
		std::vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
		m_devices.insert(m_devices.end(), devices.begin(), devices.end());
	}
}

void
ProgramBuilderFixture::addSource(char const* source_fname)
{
	QString resource_name(":/device_code/");
	resource_name += source_fname;
	QFile file(resource_name);
	if (!file.open(QIODevice::ReadOnly)) {
		throw std::runtime_error("Failed to read file: "+resource_name.toStdString());
	}
	m_sources.push_back(file.readAll());
	QByteArray const& data = m_sources.back();
	m_sourceAccessors.push_back(std::make_pair(data.data(), data.size()));
}

cl::Program
ProgramBuilderFixture::buildProgram(cl::Context const& context) const
{
	cl::Program program(context, m_sourceAccessors);

	try {
		program.build();
	} catch (cl::Error const&) {
		auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
		qDebug() << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices.front()).c_str();
		throw std::runtime_error("Failed to build OpenCL program");
	}

	return program;
}

} // namespace tests

} // namespace opencl
