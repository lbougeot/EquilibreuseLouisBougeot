#include "jaugesdecontrainte.h"
#include "constante.h"
#include <QDebug>

JaugesDeContrainte::JaugesDeContrainte(MccUldaq &_laCarte, QObject *parent) :
    QObject(parent),
    laCarte(_laCarte)
{
    double vitesse = 33000;
    valeursBrutes = new double[NB_ECHANTILLONS_PAR_CANAL * NB_CANAUX];
    ScanOption options = static_cast<ScanOption>(SO_SINGLEIO | SO_CONTINUOUS | SO_EXTCLOCK | SO_EXTTRIGGER);
    AInScanFlag flags = AINSCAN_FF_DEFAULT ;
    UlError erreur = laCarte.ulAInScan(BROCHE_JAUGE_O,BROCHE_JAUGE_A,NB_ECHANTILLONS_PAR_CANAL,&vitesse,options,flags,valeursBrutes);
    if(erreur != ERR_NO_ERROR)
        qDebug() << "JaugesDeContrainte::JaugesDeContrainte " << erreur;
}

JaugesDeContrainte::~JaugesDeContrainte()
{
    UlError erreur = laCarte.ulAInScanStop();
    if(erreur != ERR_NO_ERROR)
        qDebug() << "JaugesDeContrainte::~JaugesDeContrainte " << erreur;
}

double *JaugesDeContrainte::ObtenirMesuresBrutes(int &nbVal)
{
    double *retour = nullptr;
    ScanStatus status;
    TransferStatus transferStatus;
    UlError erreur = laCarte.ulAInScanStatus(status,transferStatus);
    if(erreur != ERR_NO_ERROR)
        qDebug() << "JaugesDeContrainte::ObtenirMesuresBrutes " << erreur;

    if(status == SS_RUNNING)
    {
        if(erreur == ERR_NO_ERROR)
        {
            nbVal = transferStatus.currentTotalCount;
            if(nbVal > NB_ECHANTILLONS_PAR_CANAL)
                nbVal = NB_ECHANTILLONS_PAR_CANAL;
            retour = valeursBrutes;
        }
    }
    return  retour;
}
