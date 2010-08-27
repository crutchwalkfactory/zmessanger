#ifndef UTIL_GIFPLAYER_H
#define UTIL_GIFPLAYER_H

#include <qwidget.h>
#include <qasyncimageio.h>

// Size: 464

class UTIL_GifPlayer : public QWidget, QImageConsumer 
{
	uint fix[66];
	
    Q_OBJECT
public:
    UTIL_GifPlayer();
    UTIL_GifPlayer(const QString& file);
    UTIL_GifPlayer(QWidget* parent, const char* name=0);
    UTIL_GifPlayer(const QString& file, QWidget* parent, const char* name=0);
    ~UTIL_GifPlayer();

    bool start(const QString& file, bool s=true);
    void restart(bool s =true);
    void startshow();
    void disableLooping();
    void disableAlpha();
    void setMinPeriod(int);
    void setMemoryLimit(int);
    int status() const;
    bool playing() const;
    QSize sizeHint() const;
    void setFullScreenMode(bool);
    void enableBuffer(bool);

signals:
    void finished();
    void loopend();

public slots:
    void pushUpdate();

protected:
    void timerEvent(QTimerEvent*);
    
    void showNextFrame();
    bool decodeOne();
    void decodeMore();
    void displayOne(int);
    void paintEvent( QPaintEvent* );
    void terminate();
    
    virtual void end();
    virtual void changed( const QRect& );
    virtual void frameDone();
    virtual void frameDone( const QPoint&, const QRect& );
    virtual void setLooping( int );
    virtual void setFramePeriod( int );
    virtual void setSize( int, int );
    
};

#endif
