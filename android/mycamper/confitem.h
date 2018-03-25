#ifndef CONFITEM_H
#define CONFITEM_H

#include <QObject>
#include <QVariant>

class ConfItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString paramName READ getName NOTIFY paramChanged)
    Q_PROPERTY(QVariant paramValue READ getValue NOTIFY paramChanged)

public:
    ConfItem(const QString &name, const QVariant &value, QObject *parent = nullptr);

    QString getName() const;
    QVariant getValue() const;

signals:
    void paramChanged();

private:
    QString m_name;
    QVariant m_value;
};

#endif // CONFITEM_H
