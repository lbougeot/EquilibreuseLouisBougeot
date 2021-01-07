#include "codeur.h"
#include <QDebug>



Codeur::Codeur(MccUldaq &_laCarte, QObject *parent):
    QObject(parent),
    laCarte(_laCarte),
    vitesse(0)
{
    connect(&timerVitesse,&QTimer::timeout,this,&Codeur::onTimerVitess_timeout);
    timerVitesse.setTimerType(Qt::PreciseTimer);
    timerVitesse.setSingleShot(true);
}

Codeur::~Codeur()
{
    disconnect(&timerVitesse,&QTimer::timeout,this,&Codeur::onTimerVitess_timeout);
}

void Codeur::LancerMesureVitesse()
{
    UlError erreur;
    erreur = laCarte.UlCClear();
    if(erreur == ERR_NO_ERROR)
        timerVitesse.start(TEMPO_VITESSE);
    else
        qDebug() << "Codeur::LancerMesureVitesse " << erreur ;
}

void Codeur::ArreterMessureVitesse()
{
    timerVitesse.stop();
    vitesse = 0 ;
}

void Codeur::onTimerVitess_timeout()
{
    long valCpt;
    UlError erreur;
    erreur = laCarte.ulCIn(valCpt);
    if(erreur == ERR_NO_ERROR)
    {
        int vitesseCalculee = valCpt / NB_POINTS_CODEUR * 60 ;
        if(vitesseCalculee != vitesse)
        {
            emit VitesseChange(vitesseCalculee);
            vitesse = vitesseCalculee;
        }
        erreur = laCarte.UlCClear();
        timerVitesse.start(TEMPO_VITESSE);
    }
     if(erreur != ERR_NO_ERROR)
        qDebug() << "Codeur::onTimerVitess_timeout " << erreur ;
}
