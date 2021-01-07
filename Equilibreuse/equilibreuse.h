#ifndef EQUILIBREUSE_H
#define EQUILIBREUSE_H

#include <QMainWindow>
#include <QLabel>
#include <QHBoxLayout>
#include <QtCharts>

#include "mcculdaq.h"
#include "jaugesdecontrainte.h"
#include "codeur.h"
#include "moteur.h"

#include "experience.h"
#include "vuegraphique.h"


QT_BEGIN_NAMESPACE
namespace Ui { class Equilibreuse; }
QT_END_NAMESPACE

class Equilibreuse : public QMainWindow
{
    Q_OBJECT

public:
    Equilibreuse(QWidget *parent = nullptr);
    ~Equilibreuse();

private slots:
    void on_actionParametrer_triggered();
    void on_actionNouvelle_Experience_triggered();
    void on_actionImporter_mesures_brutes_triggered();
    void on_actionFormat_PDF_triggered();
    void on_actionConfigurer_triggered();
    void on_buttonLancerAcquisition_clicked();
    void on_actionAffichage_en_degre_toggled(bool arg1);
    void on_actionAffichage_en_Newton_toggled(bool arg1);
    void on_buttonLancerMoteur_clicked();
    void on_buttonArretMoteur_clicked();

    void on_TimeOutAcquisitionMesures();
    void on_CodeurVitesseChange(int _vitesse);






    void on_buttonFixerConsigneVitesse_valueChanged(int value);

    void on_actionOuvrir_triggered();

    void on_actionEnregistrer_triggered();

    void on_actionFormat_CSV_triggered();

private:

    enum ETAT_SYSTEME       /// Résume les etats possibles du système
    {
        REPOS,
        PRISE_ORIGINE,
        ATTENTE_CONSIGNE_VITESSE,
        EN_DEMARRAGE,
        EN_ROTATION,
        ACQUISITION,
        SECURITE,
        RESTITUTION,
        EXPERIENCE_FIGEE
    };

    void ChangerEtatSysteme(ETAT_SYSTEME etat);
    void MettreAjourCourbe();
    void MettreAjourTableauAcquisition();
    void MettreAjourVitesseExperience();
    void LancerAcquisition();
    void ArreterAcquisition();


    Ui::Equilibreuse *ui;
    Experience *experience;
    VueGraphique *vueGraphique; /// pointeur vers la représentation graphique de l'expérience
    ETAT_SYSTEME etatDuSysteme; /// Mémorise l'état du système (automate d'état)

    QLabel labelEtatCapot;      /// Affichage de l'état du capot (ouvert ou fermé)
    QLabel labelPresenceCarte;  /// Affichage carte USB est branchée ou pas
    QLabel labelEtatSysteme;    /// Affiche l'état du système


    QChartView *chartView;
    QHBoxLayout *layout;
    QSplineSeries courbeA;
    QSplineSeries courbeO;
    QChart chart;

    MccUldaq *laCarte;
    JaugesDeContrainte *lesJauges;
    Codeur *leCodeur;
    Moteur *leMoteur;


    QTimer *timerAcquisition ;

    int vitesse;
    int nbVitesse;
    int consigne;
    int sommeVitesse[5];

    bool newton;                    /// Indique si la construction de la courbe se fait en Newton(VRAI) ou en Volt(FAUX)
    bool degre;                     /// Indique si la construction de la courbe se fait en degré(VRAI) ou en points du codeur(FAUX)
    bool decalage;                  /// Indique si le décalage à l'origine est pris en compte(VRAI)


};
#endif // EQUILIBREUSE_H
