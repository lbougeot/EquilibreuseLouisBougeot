#ifndef RAPPORTMESURES_H
#define RAPPORTMESURES_H
#include <QPdfWriter>
#include <QtCharts>
#include "experience.h"



class RapportMesures : public QPdfWriter
{
public:
    RapportMesures(Experience &_experience, QChartView &_graphique, const QString &_nomDuFichier);
    void ConstruireRapport();


private:
    void FabriquerEntete();
    void FabriquerResultats(int position  =150);
    void FabriquerGraphiques(int position =300);
    void FabriquerCommentaire(int position=800);



    Experience &experience;
    QChartView &graphique;
    QPainter  painter;

    int largeurPdf;
    int hauteurPdf;
    int margeGauche;
    int margeHaut;
    int largeurUtile;
    int padding;

    QFont fontTitre;
    QFont fontTexte;


};

#endif // RAPPORTMESURES_H
