#include "confitem.h"

ConfItem::ConfItem(const QString &name, const QVariant &value, QObject *parent) : QObject(parent)
{
    m_name = name;
    m_value = value;
}

QString ConfItem::getName() const
{
    return m_name;
}

QVariant ConfItem::getValue() const
{
    return m_value;
}
