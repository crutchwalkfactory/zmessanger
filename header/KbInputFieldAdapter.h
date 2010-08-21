#ifndef Z_INPUT_FIELD_ADAPTAR_H
#define Z_INPUT_FIELD_ADAPTAR_H

#include <qobject.h>

class ZInputEngine; 
class ZInputFieldAdapterData;
class KbEditContext; 
class QWidget;
class KbUIControlInterface;
class ZInputField;

class ZInputFieldAdapter: public QObject
{
	Q_OBJECT
public:
	ZInputFieldAdapter(QObject * parent = NULL);
	virtual ~ZInputFieldAdapter();

public:
	ZInputField* currentField() ;
	ZInputEngine* getProperInputEngine() ;
	KbEditContext* getEditContext();
	ZInputField* getKbInputField(QWidget * );
	void setInputMethods( QWidget*, const QUuid&, const QValueList<QUuid>&, int, const QString& );
	void directShowInputWidget(QWidget*, const QUuid&, int);

public:
	virtual bool eventFilter(ZInputField*, QEvent*);
	virtual bool registerField(ZInputField*);
	virtual bool unregisterField(ZInputField*); 

public:
	void emitSignalKbStateChanged(int, bool);
	KbUIControlInterface* getUIController() const;
	void setUIController(KbUIControlInterface*);

signals:
	void signalKbStateChanged(int, bool);

protected:
	ZInputFieldAdapterData * d;
}; 
#endif
