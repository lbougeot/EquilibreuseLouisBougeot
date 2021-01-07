#ifndef CODEUR_H
#define CODEUR_H

#include <QObject>
#include <QTimer>
#include "mcculdaq.h"
#include "constante.h"

class Codeur : public QObject
{
    Q_OBJECT

public:
    explicit Codeur(MccUldaq &_laCarte, QObject *parent = nullptr);
    ~Codeur();
    void LancerMesureVitesse();
    void ArreterMessureVitesse();

private slots:
    void onTimerVitess_timeout();

signals:
    void VitesseChange(const int _vitesse);

private:
    QTimer timerVitesse;
    MccUldaq &laCarte;
    int vitesse;
};

#endif // CODEUR_H
