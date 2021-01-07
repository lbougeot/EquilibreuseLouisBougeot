#ifndef JAUGESDECONTRAINTE_H
#define JAUGESDECONTRAINTE_H

#include <QObject>
#include "mcculdaq.h"

class JaugesDeContrainte : public QObject
{
    Q_OBJECT
public:
    explicit JaugesDeContrainte(MccUldaq &_laCarte, QObject *parent = nullptr);
    ~JaugesDeContrainte();
    double *ObtenirMesuresBrutes(int &nbVal);

private:
    MccUldaq &laCarte;
    int nbAcquisitions ;
    double *valeursBrutes;

signals:

};

#endif // JAUGESDECONTRAINTE_H
