#include "moteur.h"
#include <QDebug>

Moteur::Moteur(MccUldaq &_laCarte, const int _numCanal, const double _tensionMax):
    laCarte(_laCarte),
    numCanal(_numCanal),
    tensionMaxCommande(_tensionMax)
{
    UlError erreur;
    erreur = laCarte.ulAOut(numCanal,0);
    if(erreur != ERR_NO_ERROR)
        qDebug() << "Moteur::Moteur " << erreur ;
}

Moteur::~Moteur()
{
    UlError erreur;
    erreur = laCarte.ulAOut(numCanal,0);
    if(erreur != ERR_NO_ERROR)
        qDebug() << "Moteur::~Moteur " << erreur ;
}

void Moteur::FixerConsigneVitesse(const int _pourcentage)
{
    double valeurTension = tensionMaxCommande * _pourcentage / 100.0 ;
    UlError erreur;
    erreur = laCarte.ulAOut(numCanal,valeurTension);
    if(erreur != ERR_NO_ERROR)
        qDebug() << "Moteur::FixerConsigneVitesse " << erreur ;
}

