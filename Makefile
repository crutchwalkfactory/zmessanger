#PLATFORM=EZX-Z6
#PLATFORM=EZX-U9
#PLATFORM=EZX-ZN5
#PLATFORM=EZX-V8
#PLATFORM=EZX-E8
#PLATFORM=EZX-EM30

NOTMAKETOBIN=YES
APPNAME=zIM

XMPP=no

ifeq ($(PLATFORM),)
PLATFORM=EZX-Z6
endif

UPLOAD_PATH := /mmc/mmca1/bin/
TOOLPREFIX := /arm-eabi
ARMLIB     := $(TOOLPREFIX)/arm-linux-gnueabi/lib

ifeq ($(PLATFORM),EZX-Z6)
QTDIR   :=	$(TOOLPREFIX)/lib/qt-2.3.8
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-z6
DIRECTIV =	-DEZX_Z6 -DOLD_SDK -D_MainMenuFix
TARGET	=      $(APPNAME)_Z6
endif

ifeq ($(PLATFORM),EZX-V8)
QTDIR   :=	$(TOOLPREFIX)/lib/qt-2.3.8
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-v8
DIRECTIV =	-DEZX_V8 -DOLD_SDK -D_MainMenuFix
TARGET	=      $(APPNAME)_V8
endif

ifeq ($(PLATFORM),EZX-ZN5)
QTDIR   :=	$(TOOLPREFIX)/lib/qt-zn5
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-zn5
DIRECTIV =	-DEZX_ZN5 -DNEW_PLATFORM -DCUTED_PLATFORM -DFixByQT -D_MainMenuFix
#-D_ZScrollPanel
TARGET	=      $(APPNAME)_ZN5
endif

ifeq ($(PLATFORM),EZX-U9)
QTDIR   :=	$(TOOLPREFIX)/lib/qt-zn5
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-u9
DIRECTIV =	-DEZX_U9 -DNEW_PLATFORM -DCUTED_PLATFORM -DFixByQT
TARGET	=      $(APPNAME)_U9
endif

ifeq ($(PLATFORM),EZX-E8)
ARMLIB  :=	$(TOOLPREFIX)/arm-linux-gnueabi/lib_E8
QTDIR   :=	$(TOOLPREFIX)/lib/qt-e8
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-e8
DIRECTIV =	-DEZX_E8 -DSCREEN_GORIZONTAL -DNEW_PLATFORM -DFixByQT -D_MainMenuFix -D_FixQVBoxLayout
TARGET	=      $(APPNAME)_E8
endif

ifeq ($(PLATFORM),EZX-EM30)
ARMLIB  :=	$(TOOLPREFIX)/arm-linux-gnueabi/lib_E8
QTDIR   :=	$(TOOLPREFIX)/lib/qt-em30
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-em30
DIRECTIV =	-DEZX_EM30 -DSCREEN_GORIZONTAL -DNEW_PLATFORM -DFixByQT -D_MainMenuFix -D_FixQVBoxLayout
TARGET	=      $(APPNAME)_EM30
endif

ifeq ($(PLATFORM),EZX-EM35)
ARMLIB  :=	$(TOOLPREFIX)/arm-linux-gnueabi/lib_E8
QTDIR   :=	$(TOOLPREFIX)/lib/qt-em35
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-em35
DIRECTIV =	-DEZX_EM35 -DNEW_PLATFORM -DFixByQT -D_MainMenuFix -D_FixQVBoxLayout
TARGET	=      $(APPNAME)_EM35
endif

ifeq ($(PLATFORM),EZX-VE66)
ARMLIB  :=	$(TOOLPREFIX)/arm-linux-gnueabi/lib_E8
QTDIR   :=	$(TOOLPREFIX)/lib/qt-em35
EZXDIR  :=	$(TOOLPREFIX)/lib/ezx-ve66
DIRECTIV =	-DEZX_VE66 -DNEW_PLATFORM -DFixByQT -D_MainMenuFix -D_FixQVBoxLayout
TARGET	=      $(APPNAME)_VE66
endif

INCPATH	= -I. -I./comp -I./icqlib -I./dlg -I./header -I./NAPI

ifeq ($(XMPP),yes)
INCPATH	+= -I./xmpp
endif

CC	=	$(TOOLPREFIX)/bin/arm-linux-gnueabi-gcc
CXX	=	$(TOOLPREFIX)/bin/arm-linux-gnueabi-g++
LD	=	$(TOOLPREFIX)/bin/arm-linux-gnueabi-g++
LINK	=	$(TOOLPREFIX)/bin/arm-linux-gnueabi-gcc
STRIP   =       $(TOOLPREFIX)/bin/arm-linux-strip
INCPATH	+=	-I$(QTDIR)/include -I$(EZXDIR)/include -I./header -I$(TOOLPREFIX)/arm-linux-gnueabi/include
CFLAGS	=	-pipe -Wall -W -O2 -DNO_DEBUG $(INCPATH) -march=armv6j -mtune=arm1136jf-s -Os 
CXXFLAGS=	-pipe -DQWS -fno-exceptions -fno-rtti -Wall -W -DNO_DEBUG $(DIRECTIV) $(INCPATH) -march=armv6j -mtune=arm1136jf-s -Os 
LDFLAGS	=	-s
LFLAGS	=	-Wl,-rpath-link,$(QTDIR)/lib -Wl,-rpath-link,$(EZXDIR)/lib -Wl,-rpath-link,./lib
LIBS	=	$(SUBLIBS) -L$(ARMLIB) -L$(QTDIR)/lib -L$(EZXDIR)/lib -L./lib -lm -lqte-mt -lezxqtnapi -lezxappbase
MOC	=	$(QTDIR)/bin/moc
UIC	=	$(QTDIR)/bin/uic

ifeq ($(XMPP),yes)
LIBS	+=  -lgloox
endif

MAKETO=BIN/$(PLATFORM)/

TARGETS=$(TARGET)

####### Files
HEADERS =   zgui.h\
            GUI_Define.h\
            BaseDlg.h\
            zICQ.h\
            NAPI/NAPILink.h\
            NAPI/NAPIManager.h\
            comp/xTextView.h\
            comp/zEmoticon.h\
            comp/ZMyListBox.h\
            comp/ZMyApplication.h\
            comp/ZOptionItem.h\
            comp/ZContactItem.h\
            icqlib/Base64.h\
            icqlib/flap.h\
            icqlib/httpproxy.h\
            icqlib/md5.h\
            icqlib/snaccache.h\
            icqlib/socks4proxy.h\
            icqlib/socks5proxy.h\
            icqlib/tnetwork.h\
            icqlib/icqkid2.h\
            icqlib/const_strings.h\
            dlg/zChatForm.h\
            dlg/ZStatusInfo.h\
            dlg/ZImgeSelect.h\
            dlg/ZUserInfo.h\
            dlg/ZXStatusText.h\
            dlg/ZAboutDlg.h\
            dlg/ZEye.h\
            dlg/ZSettings.h\
            dlg/MessageDlg.h\
            dlg/ZAddProfile.h\
            dlg/ZHistoruView.h

ifeq ($(XMPP),yes)
HEADERS +=  \
            zXMPP.h\
            zXMPPConferenc.h
endif   
          
SOURCES =   main.cpp\
            zgui.cpp\
            BaseDlg.cpp\
            zICQ.cpp\
            NAPI/NAPILink.cpp\
            NAPI/NAPIManager.cpp\
            comp/xTextView.cpp\
            comp/zEmoticon.cpp\
            comp/ZMyListBox.cpp\
            comp/ZMyApplication.cpp\
            comp/ZOptionItem.cpp\
            comp/ZContactItem.cpp\
            icqlib/Base64.cpp\
            icqlib/flap.cpp\
            icqlib/httpproxy.cpp\
            icqlib/md5.cpp\
            icqlib/snaccache.cpp\
            icqlib/socks4proxy.cpp\
            icqlib/socks5proxy.cpp\
            icqlib/tnetwork.cpp\
            icqlib/icqkid2.cpp\
            icqlib/const_strings.cpp\
            dlg/zChatForm.cpp\
            dlg/ZStatusInfo.cpp\
            dlg/ZImgeSelect.cpp\
            dlg/ZUserInfo.cpp\
            dlg/ZXStatusText.cpp\
            dlg/ZAboutDlg.cpp\
            dlg/ZEye.cpp\
            dlg/ZSettings.cpp\
			dlg/MessageDlg.cpp\
			dlg/ZAddProfile.cpp\
            dlg/ZHistoruView.cpp

ifeq ($(XMPP),yes)
SOURCES += \
            zXMPP.cpp\
            zXMPPConferenc.cppp
endif

OBJECTS =   main.o\
            zgui.o\
            BaseDlg.o\
            zICQ.o\
            NAPI/NAPILink.o\
            NAPI/NAPIManager.o\
            comp/xTextView.o\
            comp/zEmoticon.o\
            comp/ZMyListBox.o\
            comp/ZMyApplication.o\
            comp/ZOptionItem.o\
            comp/ZContactItem.o\
            icqlib/Base64.o\
            icqlib/flap.o\
            icqlib/httpproxy.o\
            icqlib/md5.o\
            icqlib/snaccache.o\
            icqlib/socks4proxy.o\
            icqlib/socks5proxy.o\
            icqlib/tnetwork.o\
            icqlib/icqkid2.o\
            icqlib/const_strings.o\
            dlg/zChatForm.o\
            dlg/ZStatusInfo.o\
            dlg/ZImgeSelect.o\
            dlg/ZUserInfo.o\
            dlg/ZXStatusText.o\
            dlg/ZAboutDlg.o\
            dlg/ZEye.o\
            dlg/ZSettings.o\
            dlg/MessageDlg.o\
            dlg/ZAddProfile.o\
            dlg/ZHistoruView.o

ifeq ($(XMPP),yes)
OBJECTS += \
            zXMPP.o\
            zXMPPConferenc.o
endif

OBJECTS2 =  $(MAKETO)main.o\
            $(MAKETO)zgui.o\
            $(MAKETO)BaseDlg.o\
            $(MAKETO)zICQ.o\
            $(MAKETO)NAPI/NAPILink.o\
            $(MAKETO)NAPI/NAPIManager.o\
            $(MAKETO)comp/xTextView.o\
            $(MAKETO)comp/zEmoticon.o\
            $(MAKETO)comp/ZMyApplication.o\
            $(MAKETO)comp/ZMyListBox.o\
            $(MAKETO)comp/ZOptionItem.o\
            $(MAKETO)comp/ZContactItem.o\
            $(MAKETO)icqlib/Base64.o\
            $(MAKETO)icqlib/flap.o\
            $(MAKETO)icqlib/httpproxy.o\
            $(MAKETO)icqlib/md5.o\
            $(MAKETO)icqlib/snaccache.o\
            $(MAKETO)icqlib/socks4proxy.o\
            $(MAKETO)icqlib/socks5proxy.o\
            $(MAKETO)icqlib/tnetwork.o\
            $(MAKETO)icqlib/icqkid2.o\
            $(MAKETO)icqlib/const_strings.o\
            $(MAKETO)dlg/zChatForm.o\
            $(MAKETO)dlg/ZStatusInfo.o\
            $(MAKETO)dlg/ZImgeSelect.o\
            $(MAKETO)dlg/ZUserInfo.o\
            $(MAKETO)dlg/ZXStatusText.o\
            $(MAKETO)dlg/ZAboutDlg.o\
            $(MAKETO)dlg/ZEye.o\
            $(MAKETO)dlg/ZSettings.o\
            $(MAKETO)dlg/MessageDlg.o\
            $(MAKETO)dlg/ZAddProfile.o\
            $(MAKETO)dlg/ZHistoruView.o\
            $(MAKETO)zXMPP.o\
            $(MAKETO)zXMPPConferenc.o
			         
SRCMOC	= moc_zgui.cpp\
          moc_BaseDlg.cpp\
          moc_zICQ.cpp\
          NAPI/moc_NAPILink.cpp\
          NAPI/moc_NAPIManager.cpp\
          comp/moc_xTextView.cpp\
          comp/moc_zEmoticon.cpp\
          comp/moc_ZMyListBox.cpp\
          comp/moc_ZMyApplication.cpp\
          comp/moc_ZOptionItem.cpp\
          comp/moc_ZContactItem.cpp\
          icqlib/moc_Base64.cpp\
          icqlib/moc_flap.cpp\
          icqlib/moc_httpproxy.cpp\
          icqlib/moc_md5.cpp\
          icqlib/moc_snaccache.cpp\
          icqlib/moc_socks4proxy.cpp\
          icqlib/moc_socks5proxy.cpp\
          icqlib/moc_tnetwork.cpp\
          icqlib/moc_icqkid2.cpp\
          icqlib/moc_const_strings.cpp\
          dlg/moc_zChatForm.cpp\
          dlg/moc_ZStatusInfo.cpp\
          dlg/moc_ZImgeSelect.cpp\
          dlg/moc_ZUserInfo.cpp\
          dlg/moc_ZXStatusText.cpp\
          dlg/moc_ZAboutDlg.cpp\
          dlg/moc_ZEye.cpp\
          dlg/moc_ZSettings.cpp\
          dlg/moc_MessageDlg.cpp\
          dlg/moc_ZAddProfile.cpp\
          dlg/moc_ZHistoruView.cpp

ifeq ($(XMPP),yes)
SRCMOC += \
          moc_zXMPP.cpp\
          moc_zXMPPConferenc.cpp
endif

SRCMOC2	= $(MAKETO)moc_zgui.cpp\
          $(MAKETO)moc_BaseDlg.cpp\
          $(MAKETO)moc_zICQ.cpp\
          $(MAKETO)NAPI/moc_NAPILink.cpp\
          $(MAKETO)NAPI/moc_NAPIManager.cpp\
          $(MAKETO)comp/moc_xTextView.cpp\
          $(MAKETO)comp/moc_zEmoticon.cpp\
          $(MAKETO)comp/moc_ZMyListBox.cpp\
          $(MAKETO)comp/moc_ZMyApplication.cpp\
          $(MAKETO)comp/moc_ZOptionItem.cpp\
          $(MAKETO)comp/moc_ZContactItem.cpp\
          $(MAKETO)icqlib/moc_Base64.cpp\
          $(MAKETO)icqlib/moc_flap.cpp\
          $(MAKETO)icqlib/moc_httpproxy.cpp\
          $(MAKETO)icqlib/moc_md5.cpp\
          $(MAKETO)icqlib/moc_snaccache.cpp\
          $(MAKETO)icqlib/moc_socks4proxy.cpp\
          $(MAKETO)icqlib/moc_socks5proxy.cpp\
          $(MAKETO)icqlib/moc_tnetwork.cpp\
          $(MAKETO)icqlib/moc_icqkid2.cpp\
          $(MAKETO)icqlib/moc_const_strings.cpp\
          $(MAKETO)dlg/moc_zChatForm.cpp\
          $(MAKETO)dlg/moc_ZStatusInfo.cpp\
          $(MAKETO)dlg/moc_ZImgeSelect.cpp\
          $(MAKETO)dlg/moc_ZUserInfo.cpp\
          $(MAKETO)dlg/moc_ZXStatusText.cpp\
          $(MAKETO)dlg/moc_ZAboutDlg.cpp\
          $(MAKETO)dlg/moc_ZEye.cpp\
          $(MAKETO)dlg/moc_ZSettings.cpp\
          $(MAKETO)dlg/moc_MessageDlg.cpp\
          $(MAKETO)dlg/moc_ZAddProfile.cpp\
          $(MAKETO)dlg/moc_ZHistoruView.cpp\
          $(MAKETO)moc_zXMPP.cpp\
          $(MAKETO)moc_zXMPPConferenc.cpp
	    
OBJMOC	= moc_zgui.o\
          moc_BaseDlg.o\
          moc_zICQ.o\
          NAPI/moc_NAPILink.o\
          NAPI/moc_NAPIManager.o\
          comp/moc_xTextView.o\
          comp/moc_zEmoticon.o\
          comp/moc_ZMyListBox.o\
          comp/moc_ZMyApplication.o\
          comp/moc_ZOptionItem.o\
          comp/moc_ZContactItem.o\
          icqlib/moc_Base64.o\
          icqlib/moc_flap.o\
          icqlib/moc_httpproxy.o\
          icqlib/moc_md5.o\
          icqlib/moc_snaccache.o\
          icqlib/moc_socks4proxy.o\
          icqlib/moc_socks5proxy.o\
          icqlib/moc_tnetwork.o\
          icqlib/moc_icqkid2.o\
          icqlib/moc_const_strings.o\
          dlg/moc_zChatForm.o\
          dlg/moc_ZStatusInfo.o\
          dlg/moc_ZImgeSelect.o\
          dlg/moc_ZUserInfo.o\
          dlg/moc_ZXStatusText.o\
          dlg/moc_ZAboutDlg.o\
          dlg/moc_ZEye.o\
          dlg/moc_ZSettings.o\
          dlg/moc_MessageDlg.o\
          dlg/moc_ZAddProfile.o\
          dlg/moc_ZHistoruView.o

ifeq ($(XMPP),yes)
OBJMOC += \
          moc_zXMPP.o\
          moc_zXMPPConferenc.o
endif
       
OBJMOC2	= $(MAKETO)moc_zgui.o\
          $(MAKETO)moc_BaseDlg.o\
          $(MAKETO)moc_zICQ.o\
          $(MAKETO)NAPI/moc_NAPILink.o\
          $(MAKETO)NAPI/moc_NAPIManager.o\
          $(MAKETO)comp/moc_xTextView.o\
          $(MAKETO)comp/moc_zEmoticon.o\
          $(MAKETO)comp/moc_ZMyListBox.o\
          $(MAKETO)comp/moc_ZMyApplication.o\
          $(MAKETO)comp/moc_ZOptionItem.o\
          $(MAKETO)comp/moc_ZContactItem.o\
          $(MAKETO)icqlib/moc_Base64.o\
          $(MAKETO)icqlib/moc_flap.o\
          $(MAKETO)icqlib/moc_httpproxy.o\
          $(MAKETO)icqlib/moc_md5.o\
          $(MAKETO)icqlib/moc_snaccache.o\
          $(MAKETO)icqlib/moc_socks4proxy.o\
          $(MAKETO)icqlib/moc_socks5proxy.o\
          $(MAKETO)icqlib/moc_tnetwork.o\
          $(MAKETO)icqlib/moc_icqkid2.o\
          $(MAKETO)icqlib/moc_const_strings.o\
          $(MAKETO)dlg/moc_zChatForm.o\
          $(MAKETO)dlg/moc_ZStatusInfo.o\
          $(MAKETO)dlg/moc_ZImgeSelect.o\
          $(MAKETO)dlg/moc_ZUserInfo.o\
          $(MAKETO)dlg/moc_ZXStatusText.o\
          $(MAKETO)dlg/moc_ZAboutDlg.o\
          $(MAKETO)dlg/moc_ZEye.o\
          $(MAKETO)dlg/moc_ZSettings.o\
          $(MAKETO)dlg/moc_MessageDlg.o\
          $(MAKETO)dlg/moc_ZAddProfile.o\
          $(MAKETO)dlg/moc_ZHistoruView.o\
          $(MAKETO)moc_zXMPP.o\
          $(MAKETO)moc_zXMPPConferenc.o
		        
#--------- Implicit rules
ifneq ($(NOTMAKETOBIN),YES)
.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$(MAKETO)$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$(MAKETO)$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$(MAKETO)$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$(MAKETO)$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$(MAKETO)$@" "$<"
endif
#--------- Build rules -----------------------------

all: $(TARGET)

ifeq ($(NOTMAKETOBIN),YES)
$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJMOC) $(LIBS)
		$(STRIP) -s $(TARGET)
else
$(TARGET): $(UICDECLS) $(OBJECTS) $(OBJMOC) 
	$(LINK) $(LFLAGS) -o $(MAKETO)$(TARGET) $(OBJECTS2) $(OBJMOC2) $(LIBS)
		$(STRIP) -s $(MAKETO)$(TARGET)
endif

#ifeq ($(PLATFORM),EZX-V8)
moc: $(SRCMOC)
#else
#moc: $(SRCMOC2)
#endif

clean:
	-rm -f $(OBJECTS) $(OBJMOC) $(SRCMOC) $(UICIMPLS) $(UICDECLS)
	-rm -f *~ core
	$(RM) *.o
	$(RM) moc_*.cpp

#--------- moc's -----------------------------
moc_zgui.o: moc_zgui.cpp zgui.h
moc_zgui.cpp: zgui.h
	$(MOC) zgui.h -o moc_zgui.cpp
  
moc_BaseDlg.o: moc_BaseDlg.cpp BaseDlg.h
moc_BaseDlg.cpp: BaseDlg.h
	$(MOC) BaseDlg.h -o moc_BaseDlg.cpp

moc_zICQ.o: moc_zICQ.cpp zICQ.h
moc_zICQ.cpp: zICQ.h
	$(MOC) zICQ.h -o moc_zICQ.cpp

NAPI/moc_NAPILink.o: NAPI/moc_NAPILink.cpp ./NAPI/NAPILink.h
NAPI/moc_NAPILink.cpp: ./NAPI/NAPILink.h
	$(MOC) ./NAPI/NAPILink.h -o NAPI/moc_NAPILink.cpp	

NAPI/moc_NAPIManager.o: NAPI/moc_NAPIManager.cpp ./NAPI/NAPIManager.h
NAPI/moc_NAPIManager.cpp: ./NAPI/NAPIManager.h
	$(MOC) ./NAPI/NAPIManager.h -o NAPI/moc_NAPIManager.cpp

comp/moc_xTextView.o: comp/moc_xTextView.cpp ./comp/xTextView.h
comp/moc_xTextView.cpp: ./comp/xTextView.h
	$(MOC) ./comp/xTextView.h -o comp/moc_xTextView.cpp	
	
comp/moc_zEmoticon.o: comp/moc_zEmoticon.cpp ./comp/zEmoticon.h
comp/moc_zEmoticon.cpp: ./comp/zEmoticon.h
	$(MOC) ./comp/zEmoticon.h -o comp/moc_zEmoticon.cpp	

comp/moc_ZMyListBox.o: comp/moc_ZMyListBox.cpp ./comp/ZMyListBox.h
comp/moc_ZMyListBox.cpp: ./comp/ZMyListBox.h
	$(MOC) ./comp/ZMyListBox.h -o comp/moc_ZMyListBox.cpp	

comp/moc_ZMyApplication.o: comp/moc_ZMyApplication.cpp ./comp/ZMyApplication.h
comp/moc_ZMyApplication.cpp: ./comp/ZMyApplication.h
	$(MOC) ./comp/ZMyApplication.h -o comp/moc_ZMyApplication.cpp

comp/moc_ZOptionItem.o: comp/moc_ZOptionItem.cpp ./comp/ZOptionItem.h
comp/moc_ZOptionItem.cpp: ./comp/ZOptionItem.h
	$(MOC) ./comp/ZOptionItem.h -o comp/moc_ZOptionItem.cpp

comp/moc_ZContactItem.o: comp/moc_ZContactItem.cpp ./comp/ZContactItem.h
comp/moc_ZContactItem.cpp: ./comp/ZContactItem.h
	$(MOC) ./comp/ZContactItem.h -o comp/moc_ZContactItem.cpp

icqlib/moc_Base64.o: icqlib/moc_Base64.cpp ./icqlib/Base64.h
icqlib/moc_Base64.cpp: ./icqlib/Base64.h
	$(MOC) ./icqlib/Base64.h -o icqlib/moc_Base64.cpp

icqlib/moc_flap.o: icqlib/moc_flap.cpp ./icqlib/flap.h
icqlib/moc_flap.cpp: ./icqlib/flap.h
	$(MOC) ./icqlib/flap.h -o icqlib/moc_flap.cpp

icqlib/moc_httpproxy.o: icqlib/moc_httpproxy.cpp ./icqlib/httpproxy.h
icqlib/moc_httpproxy.cpp: ./icqlib/httpproxy.h
	$(MOC) ./icqlib/httpproxy.h -o icqlib/moc_httpproxy.cpp
	
icqlib/moc_md5.o: icqlib/moc_md5.cpp ./icqlib/md5.h
icqlib/moc_md5.cpp: ./icqlib/md5.h
	$(MOC) ./icqlib/md5.h -o icqlib/moc_md5.cpp
	
icqlib/moc_snaccache.o: icqlib/moc_snaccache.cpp ./icqlib/snaccache.h
icqlib/moc_snaccache.cpp: ./icqlib/snaccache.h
	$(MOC) ./icqlib/snaccache.h -o icqlib/moc_snaccache.cpp
	
icqlib/moc_socks4proxy.o: icqlib/moc_socks4proxy.cpp ./icqlib/socks4proxy.h
icqlib/moc_socks4proxy.cpp: ./icqlib/socks4proxy.h
	$(MOC) ./icqlib/socks4proxy.h -o icqlib/moc_socks4proxy.cpp
	
icqlib/moc_socks5proxy.o: icqlib/moc_socks5proxy.cpp ./icqlib/socks5proxy.h
icqlib/moc_socks5proxy.cpp: ./icqlib/socks5proxy.h
	$(MOC) ./icqlib/socks5proxy.h -o icqlib/moc_socks5proxy.cpp
	
icqlib/moc_tnetwork.o: icqlib/moc_tnetwork.cpp ./icqlib/tnetwork.h
icqlib/moc_tnetwork.cpp: ./icqlib/tnetwork.h
	$(MOC) ./icqlib/tnetwork.h -o icqlib/moc_tnetwork.cpp
	
icqlib/moc_icqkid2.o: icqlib/moc_icqkid2.cpp ./icqlib/icqkid2.h
icqlib/moc_icqkid2.cpp: ./icqlib/icqkid2.h
	$(MOC) ./icqlib/icqkid2.h -o icqlib/moc_icqkid2.cpp

icqlib/moc_const_strings.o: icqlib/moc_const_strings.cpp ./icqlib/const_strings.h
icqlib/moc_const_strings.cpp: ./icqlib/const_strings.h
	$(MOC) ./icqlib/const_strings.h -o icqlib/moc_const_strings.cpp
	
dlg/moc_zChatForm.o: dlg/moc_zChatForm.cpp ./dlg/zChatForm.h
dlg/moc_zChatForm.cpp: ./dlg/zChatForm.h
	$(MOC) ./dlg/zChatForm.h -o dlg/moc_zChatForm.cpp	
	
dlg/moc_ZStatusInfo.o: dlg/moc_ZStatusInfo.cpp ./dlg/ZStatusInfo.h
dlg/moc_ZStatusInfo.cpp: ./dlg/ZStatusInfo.h
	$(MOC) ./dlg/ZStatusInfo.h -o dlg/moc_ZStatusInfo.cpp	
	
dlg/moc_ZImgeSelect.o: dlg/moc_ZImgeSelect.cpp ./dlg/ZImgeSelect.h
dlg/moc_ZImgeSelect.cpp: ./dlg/ZImgeSelect.h
	$(MOC) ./dlg/ZImgeSelect.h -o dlg/moc_ZImgeSelect.cpp		
	
dlg/moc_ZUserInfo.o: dlg/moc_ZUserInfo.cpp ./dlg/ZUserInfo.h
dlg/moc_ZUserInfo.cpp: ./dlg/ZUserInfo.h
	$(MOC) ./dlg/ZUserInfo.h -o dlg/moc_ZUserInfo.cpp

dlg/moc_ZXStatusText.o: dlg/moc_ZXStatusText.cpp ./dlg/ZXStatusText.h
dlg/moc_ZXStatusText.cpp: ./dlg/ZXStatusText.h
	$(MOC) ./dlg/ZXStatusText.h -o dlg/moc_ZXStatusText.cpp		
		
dlg/moc_ZAboutDlg.o: dlg/moc_ZAboutDlg.cpp ./dlg/ZAboutDlg.h
dlg/moc_ZAboutDlg.cpp: ./dlg/ZAboutDlg.h
	$(MOC) ./dlg/ZAboutDlg.h -o dlg/moc_ZAboutDlg.cpp

dlg/moc_ZEye.o: dlg/moc_ZEye.cpp ./dlg/ZEye.h
dlg/moc_ZEye.cpp: ./dlg/ZEye.h
	$(MOC) ./dlg/ZEye.h -o dlg/moc_ZEye.cpp

dlg/moc_ZSettings.o: dlg/moc_ZSettings.cpp ./dlg/ZSettings.h
dlg/moc_ZSettings.cpp: ./dlg/ZSettings.h
	$(MOC) ./dlg/ZSettings.h -o dlg/moc_ZSettings.cpp

dlg/moc_MessageDlg.o: dlg/moc_MessageDlg.cpp ./dlg/MessageDlg.h
dlg/moc_MessageDlg.cpp: ./dlg/MessageDlg.h
	$(MOC) ./dlg/MessageDlg.h -o dlg/moc_MessageDlg.cpp

dlg/moc_ZAddProfile.o: dlg/moc_ZAddProfile.cpp ./dlg/ZAddProfile.h
dlg/moc_ZAddProfile.cpp: ./dlg/ZAddProfile.h
	$(MOC) ./dlg/ZAddProfile.h -o dlg/moc_ZAddProfile.cpp

dlg/moc_ZHistoruView.o: dlg/ZHistoruView.cpp ./dlg/ZHistoruView.h
dlg/moc_ZHistoruView.cpp: ./dlg/ZHistoruView.h
	$(MOC) ./dlg/ZHistoruView.h -o dlg/moc_ZHistoruView.cpp
	
ifeq ($(XMPP),yes)
moc_zXMPP.o: moc_zXMPP.cpp zXMPP.h
moc_zXMPP.cpp: zXMPP.h
	$(MOC) zXMPP.h -o moc_zXMPP.cpp

moc_zXMPPConferenc.o: moc_zXMPPConferenc.cpp zXMPPConferenc.h
moc_zXMPPConferenc.cpp: zXMPPConferenc.h
	$(MOC) zXMPPConferenc.h -o moc_zXMPPConferenc.cpp
endif
#----------------------------------------------
install: $(TARGETS)
	ncftpput -u root -p "" 192.168.16.2 $(UPLOAD_PATH) $(TARGET)
