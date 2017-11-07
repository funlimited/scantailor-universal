#ifndef HOTKEYSMANAGER_H
#define HOTKEYSMANAGER_H

#include <Qt>
#include <QString>
#include <QMouseEvent>
#include <QSettings>
#include <QObject>

enum KeyType {
    DefaultValueKT = 0,
    KeysAllowed = 1,
    ModifierAllowed = 2,
    KeysAndModifiers = KeysAllowed|ModifierAllowed
};

enum HotKeyType {
    DefaultValueHK = 0,
    HotKey,
    MouseClick,
    MouseHold,
    MouseWheel,
    MouseDoubleCLick
};

enum HotKeysId {
    DefaultValueId,
    ProjectNew,
    ProjectOpen,
    ProjectSave,
    ProjectSaveAs,
    ProjectClose,
    AppQuit,
    StageFixOrientation,
    StageSplitPages,
    StageDeskew,
    StageSelectContent,
    StageMargins,
    StageOutput,
    PageFirst,
    PageLast,
    PagePrev,
    PageNext,
    DeskewChange,
    DeskewChangePrec,
    ContentMove,
    ContentMoveAxes,
    ContentStretch,
    ContentInsert,
    ContentDelete,
    ContentMoveUp,
    ContentMoveDown,
    ContentMoveLeft,
    ContentMoveRight,
    PageViewZoomIn,
    PageViewZoomOut,
    PageViewDisplayOriginal,
    PageViewMoveNoRestrictions,
    ZoneRectangle,
    ZoneMove,
    ZoneMoveVertically,
    ZoneMoveHorizontally,
    ZonePaste,
    ZoneClone,
    ZoneDelete,
    ZoneCancel,
    DewarpingMoveVertically,
    DewarpingMoveHorizontally,
    DewarpingDeletePoint
};

struct HotKeySequence {
    HotKeySequence(){}
    HotKeySequence(Qt::KeyboardModifiers modifier, QVector<Qt::Key> keys):
        m_modifierSequence(modifier), m_keySequence(keys) {}
    HotKeySequence(Qt::KeyboardModifiers modifier, Qt::Key key):
        m_modifierSequence(modifier), m_keySequence(QVector<Qt::Key>(1, key)) {}

    Qt::KeyboardModifiers m_modifierSequence;
    QVector<Qt::Key> m_keySequence;
};

class HotKeyInfo {
public:
    HotKeyInfo() {}
    HotKeyInfo(const HotKeysId id, const QString& title, const KeyType editorType, const HotKeyType displayType,
               const HotKeySequence sequence):
        HotKeyInfo(id, title, editorType, displayType, QVector<HotKeySequence>(1,sequence)) {}

    HotKeyInfo(const HotKeysId id, const QString& title, const KeyType editorType, const HotKeyType displayType,
               const QVector<HotKeySequence>& sequences):
        m_id(id), m_title(title), m_editorType(editorType), m_displayType(displayType),
        m_sequences(sequences) {}

    const HotKeysId id() const { return m_id; }
    const QString& title() const { return m_title; }
    const HotKeyType displayType() const { return m_displayType; }
    const KeyType editorType() const { return m_editorType; }
    const QVector<HotKeySequence>& sequences() const { return m_sequences; }
    QVector<HotKeySequence>& sequences() { return m_sequences; }
    static const QString hotKeyTypeToString(const HotKeyType& val);
private:
    HotKeysId m_id;
    QString m_title;
    KeyType m_editorType;
    HotKeyType m_displayType;
    QVector<HotKeySequence> m_sequences;
};

class HotKeyGroup {
public:
    HotKeyGroup() {}
    HotKeyGroup(const QString& groupName, const QString& groupTitle):
        m_groupName(groupName), m_groupTitle(groupTitle) {}
    bool load(const QSettings& settings);
    void save(QSettings& settings) const;
    void setHotKeys(const QVector<HotKeyInfo>& data) {m_hotKeys = data; }
    const QVector<HotKeyInfo>& hotKeys() const { return m_hotKeys; }
    QVector<HotKeyInfo>& hotKeys() { return m_hotKeys; }
    const QString& id() const { return m_groupName; }
    const QString& title() const { return m_groupTitle; }
private:
    QString m_groupName;
    QString m_groupTitle;
    QVector<HotKeyInfo> m_hotKeys;
};

class QHotKeys {
public:
    void resetToDefaults();
    bool load(QSettings *_settings = nullptr);
    void save(QSettings* settings= nullptr) const;
    const QString toDisplayableText() const;
    const HotKeyInfo *get( const HotKeysId& id) const;
    bool replace(const HotKeysId &id, const HotKeyInfo& new_val);
    static const QString modifiersToString(const Qt::KeyboardModifiers modifiers);
    static const QString keysToString(const QVector<Qt::Key>& keys);
    static const QString hotkeysToString(const Qt::KeyboardModifiers modifiers, const QVector<Qt::Key>& keys);
private:
    QVector<HotKeyGroup> m_data;
};

#endif // HOTKEYSMANAGER_H