#ifndef QVALIDATEDTEXTEDIT_H
#define QVALIDATEDTEXTEDIT_H

#include <QTextEdit>

class QValidator;
class QValidatedTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit QValidatedTextEdit(QWidget *parent);
    void clear();
    void setCheckValidator(const QValidator *v);
    bool isValid();

    bool getIsValidManually() const;
    void setIsValidManually(bool value);
	
    bool getEmptyIsValid() const;
    void setEmptyIsValid(bool value);

protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

private:
    bool valid;
    const QValidator *checkValidator;
    bool emptyIsValid;
    bool isValidManually;

public Q_SLOTS:
    void checkValidity();
	void setValid(bool valid);
    void setEnabled(bool enabled);

Q_SIGNALS:

private Q_SLOTS:
    void markValid();
};

#endif // QVALIDATEDTEXTEDIT_H
