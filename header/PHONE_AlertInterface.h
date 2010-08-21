

// Copyright (c) 26-Apr-07 - 2008 Motorola, Inc. All rights reserved.


#ifndef PHONE_ALERTINTERFACE_H
#define PHONE_ALERTINTERFACE_H

#ifndef __cplusplus
#error "This is a C++ header file; it requires C++ to compile."
#endif

#include <qobject.h>
#include <qobjectdefs.h>
#include <qstring.h>
#include <unistd.h>

enum ALERT_APPLICATION_T  
{        
    ALERT_APPLICATION_UNKNOWN = 0,   
    ALERT_APPLICATION_INCOMINGCALL_LINE1, 
    ALERT_APPLICATION_INCOMINGCALL_LINE2, 
    ALERT_APPLICATION_VIDEO_CALL,
    ALERT_APPLICATION_PHONE_GENERAL, 
    ALERT_APPLICATION_MESSAGING,  
    ALERT_APPLICATION_ALARM,   
    ALERT_APPLICATION_CALENDAR, 
    ALERT_APPLICATION_STK,
    ALERT_APPLICATION_EMAIL, 
    ALERT_APPLICATION_MISSED_EVENT,   
    ALERT_APPLICATION_PLAY_NORMAL_SOUND, 
    ALERT_APPLICATION_USSD_ALERT,               
    ALERT_CHARGE_PLUG, 
    ALERT_CHARGE_PLUGIN, 
    ALERT_CHARGE_PLUGOUT,
    ALERT_CHARGE_AUDIBLE_TIMER, 
    ALERT_POWER_ON,  
    ALERT_POWER_OFF,  
    ALERT_JAVA_MESSAGE, 
    ALERT_LOW_POWER, 
    ALERT_APPLICATION_INSTANT_MESSAGE,
    ALERT_APPLICATION_VOICE_MAIL,                           
    ALERT_APPLICATION_PTT_RING ,  
    ALERT_APPLICATION_PTT_ALERT ,   
    ALERT_APPLICATION_PTT_REMINDER ,
    ALERT_APPLICATION_PTT_AUDIO_SYSTEMDEV ,  
    ALERT_CHARGING_EIGHTY_PERCENT,
    ALERT_CHARGING_COMPLETE,

    ALERT_APPLICATION_HEADSET_NOTIFY,

    ALERT_APPLICATION_TOTAL,                              

};

enum
{
     SYSTEM_AUDIO_DEVICE = 0,
     NORMAL_AUDIO_DEVICE,
};

/*
SETUP_PlayAlert::AlertFinished()		 // FUNC
SETUP_PlayAlert::className() const		 // FUNC
SETUP_PlayAlert::event(QEvent*)		 // FUNC
SETUP_PlayAlert::ForceOnce(bool)		 // FUNC+
SETUP_PlayAlert::GetPlayStatus()		 // FUNC+
SETUP_PlayAlert::GetSpecialAlertStyle()		 // FUNC+
SETUP_PlayAlert::GetTotalTicks()		 // FUNC
SETUP_PlayAlert::installSignal()		 // FUNC
SETUP_PlayAlert::metaObject() const		 // FUNC
SETUP_PlayAlert::metaObj		 // OBJECT
SETUP_PlayAlert::PlayAlert()		 // FUNC+
SETUP_PlayAlert::PlayVRAlert(QString const&, VR_PLAYTTS_T)		 // FUNC+
SETUP_PlayAlert::PreviewVibration(ENUM_VIBRATE_PATTERN_E)		 // FUNC+
SETUP_PlayAlert::PreviewVibrationOff()		 // FUNC+
SETUP_PlayAlert::sendBSIRtoAtCommand(unsigned short)		 // FUNC
SETUP_PlayAlert::setAppType(int)		 // FUNC+
SETUP_PlayAlert::setDuration(int)		 // FUNC+
SETUP_PlayAlert::SetFunlightPattern(QString const&)		 // FUNC+
SETUP_PlayAlert::SetSpecialAlertStyle(ENUM_ALERT_STYLE const&)		 // FUNC+
SETUP_PlayAlert::SetSpecialTone(QString const&)		 // FUNC+
SETUP_PlayAlert::~SETUP_PlayAlert()		 // FUNC+
SETUP_PlayAlert::~SETUP_PlayAlert()		 // FUNC+
SETUP_PlayAlert::~SETUP_PlayAlert()		 // FUNC+
SETUP_PlayAlert::SETUP_PlayAlert(int, QString const&, int, QObject*, char const*)		 // FUNC+
SETUP_PlayAlert::SETUP_PlayAlert(int, QString const&, int, QObject*, char const*)		 // FUNC+
SETUP_PlayAlert::setVolume(int)		 // FUNC+
SETUP_PlayAlert::slotKeyEvent(QCString const&, QArray<char> const&)		 // FUNC
SETUP_PlayAlert::slotsystemmessage(QCString const&, QArray<char> const&)	 // FUNC
SETUP_PlayAlert::staticMetaObject()		 // FUNC
SETUP_PlayAlert::StopAlert()		 // FUNC+
SETUP_PlayAlert::stopChargingLight()		 // FUNC+
SETUP_PlayAlert::tr(char const*, char const*)		 // FUNC
SETUP_PlayAlert::tr(char const*)		 // FUNC
vtable for QArray<char>@@VERSION		 // OBJECT
vtable for QCString@@VERSION		 // OBJECT
vtable for SETUP_PlayAlert@@VERSION		 // OBJECT
*/

enum ENUM_ALERT_STYLE {
  ALERT_STYLE_RING_ONLY_EV, ALERT_STYLE_RING_AND_VIBRATE_EV, ALERT_STYLE_VIBRATE_THEN_RING_EV, ALERT_STYLE_VIBRATE_ONLY_EV,
  ALERT_STYLE_SILENT_EV
};

class PHONE_PlayAlert;

class SETUP_PlayAlert : public QObject
{
Q_OBJECT

public:

    SETUP_PlayAlert(int app, const QString &special = QString::null, int dt = -1, QObject *p = NULL, const char *name = NULL); //

    virtual ~ SETUP_PlayAlert();//

    virtual void SetSpecialTone(const QString &special);//
    //virtual void SetSpecialAlertStyle( int profile );//
    virtual void SetSpecialAlertStyle(ENUM_ALERT_STYLE const&);//?

    virtual int  GetSpecialAlertStyle();//

    virtual void SetFunlightPattern(const QString &flpattern);//
    virtual void stopChargingLight();//

    virtual void setAppType(int appType);//

    //void setDevType(int devType, int devSubtype);

    virtual void setDuration(int dt);//

    virtual void setVolume(int vol);//

    virtual void PlayAlert();//

    virtual void StopAlert();//

    virtual int  GetPlayStatus();//

    virtual void ForceOnce(bool bOnce=true);//

    void PreviewVibration(int ve);//

    void PreviewVibrationOff(void);//

    virtual int GetTotalTicks();//?? SETUP_PlayAlert::GetTotalTicks()		 // FUNC

    void PlayVRAlert(const QString & sText, int vrAppType=7);	//

	bool AlertFinished(); //??? SETUP_PlayAlert::AlertFinished()		 // FUNC

   // pid_t getAlertPID(void);

   // void runCheckProcessStatus(void);

  signals: 

    void sigAlertFinished();

private: 	
    PHONE_PlayAlert *d;
};

/*
class PHONE_PlayAlert;

class PHONE_AlertInterface : public QObject
{
Q_OBJECT

public:

    PHONE_AlertInterface(int app, const QString &special = QString::null, int dt = -1, QObject *p = NULL, const char *name = NULL); 

    virtual ~ PHONE_AlertInterface();

    virtual void setSpecialTone(const QString &special);
    virtual void setSpecialAlertStyle( int profile );

    virtual int  getSpecialAlertStyle();

    virtual void setFunlightPattern(const QString &flpattern);
    virtual void stopChargingLight();

    virtual void setAppType(int appType);

    void setDevType(int devType, int devSubtype);

    virtual void setDuration(int dt);

    virtual void setVolume(int vol);

    virtual void playAlert();

    virtual void stopAlert();

    virtual int  getPlayStatus();

    virtual void forceOnce(bool bOnce=true);

    void previewVibration(int ve);

    void previewVibrationOff(void);

    void playVRAlert(const QString & sText, int vrAppType=7);	

    pid_t getAlertPID(void);

    void runCheckProcessStatus(void);

  signals: 

    void sigAlertFinished();

private: 	
    PHONE_PlayAlert *d;
};

extern "C" PHONE_AlertInterface * PHONE_CreateAlertInterface(int app);

extern "C" void PHONE_DestroyAlertInterface(PHONE_AlertInterface* p);

typedef  PHONE_AlertInterface *(*PHONE_CreateAlertInterface_T)(int);

typedef  void (*PHONE_DestroyAlertInterface_T)(PHONE_AlertInterface*);
*/
#endif 
