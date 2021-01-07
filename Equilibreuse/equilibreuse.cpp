/**
 * @file equilibreuse.cpp
 * @date Création 19 janvier 2018
 * @date Mise à jour 5 janvier 2021
 * @version USB-202 1.0
 * @author Philippe CRUCHET
 * @details Classe Equilibreuse : Classe principale de l'application
 */

#include "equilibreuse.h"
#include "ui_equilibreuse.h"

#include "dialogparametrageequilibreuse.h"
#include "dialogconfigurerexperience.h"
#include "rapportmesures.h"

#include <QFileInfo>
#include <QSettings>
#include <QFileDialog>
/**
 * @brief Equilibreuse::Equilibreuse
 * @param parent
 * @details Constructeur de la classe, Initialise l'IHM et les attributs de la classe
 *          Positionne le sytème en mode REPOS si la catre est présente ou RESTITUTION
 *          si absence de carte.

 */
Equilibreuse::Equilibreuse(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Equilibreuse)
    , experience(nullptr)
    , vueGraphique(nullptr)
    , chartView(nullptr)
    , laCarte(nullptr)
    , lesJauges(nullptr)
    , leCodeur(nullptr)
    , leMoteur(nullptr)
    , timerAcquisition(nullptr)
    , newton(true)
    , degre(true)

{
    ui->setupUi(this);


    // intialisation de la barre de status
    ui->statusBar->addPermanentWidget(&labelEtatSysteme);
    ui->statusBar->addPermanentWidget(&labelEtatCapot);
    ui->statusBar->addPermanentWidget(&labelPresenceCarte);
    ui->statusBar->showMessage(tr("Equilibreuse MC USB-202 Ver 1.0 PCT"), 5000);

    ui->actionAffichage_en_Newton->setChecked(newton);
    ui->actionAffichage_en_degre->setChecked(degre);
    ui->actionDecalage_origine->setChecked(decalage);


    layout = new QHBoxLayout(ui->widgetCourbes);

    if(laCarte == nullptr)
    {
        laCarte = new MccUldaq;
        if(!laCarte->VerifierPresenceCarte())
        {
            labelPresenceCarte.setText(" | La carte n'est pas connectée ");
            ChangerEtatSysteme( RESTITUTION );
            ui->actionNouvelle_Experience->setDisabled(true);
        }
        else
        {
            labelPresenceCarte.setText(" | La carte est connectée ");
            ChangerEtatSysteme(REPOS);
            ui->actionNouvelle_Experience->setEnabled(true);
        }

    }
}

/**
 * @brief Equilibreuse::~Equilibreuse
 * @details Destructeur de la classe
 */
Equilibreuse::~Equilibreuse()
{
    delete ui;
    if(leMoteur != nullptr)
        delete leMoteur;
    if(laCarte != nullptr)
        delete laCarte;
    if(chartView != nullptr)
        delete chartView;
    if(experience != nullptr)
        delete experience;
    delete ui;
}

/* ----- Action du menu ------ */

/**
 * @brief Equilibreuse::on_actionNouvelle_Experience_triggered
 * @details Création d'une nouvelle expérience, la carte doit être présente pour
 *          faire une nouvelle expérience.
 *          Le mode normal est le mode PRISE_ORIGINE
 *          pour la simulation le mode passe directement à ATTENTE_CONSIGNE_VITESSE
 */
void Equilibreuse::on_actionNouvelle_Experience_triggered()
{
    if(experience !=nullptr)
        delete experience;
    experience = new Experience;
    MettreAjourCourbe();

    ChangerEtatSysteme(ATTENTE_CONSIGNE_VITESSE);  // il n'y a pas de prise d'origine
}

/**
 * @brief Equilibreuse::on_actionOuvrir_triggered
 * @details Ouvre un fichier .equ contenant les courbes d'une précédente expérience.
 *          Affiche les courbes et effectue la mis à jour de leurs caractéristiques.
 */
void Equilibreuse::on_actionOuvrir_triggered()
{
    if(experience != nullptr)
        delete experience;
    experience = new Experience(this);


    QFileDialog fichierDlg(this);
    QString leFichier;
    fichierDlg.setAcceptMode(QFileDialog::AcceptOpen);
    fichierDlg.setWindowTitle("Ouverture d'une expérience d'équilibrage");
    fichierDlg.setViewMode(QFileDialog::Detail);
    fichierDlg.setOption(QFileDialog::DontResolveSymlinks);
    fichierDlg.setNameFilter("Fichier d'équilibrage (*equ)");
    if(fichierDlg.exec() && fichierDlg.selectedFiles().size() != 0)
    {
        leFichier = fichierDlg.selectedFiles()[0];
        if (!leFichier.isEmpty())
        {
            experience->LireCourbes(leFichier);
            MettreAjourCourbe();
            MettreAjourTableauAcquisition();
            MettreAjourVitesseExperience();

            ChangerEtatSysteme(RESTITUTION);
        }
    }
}

/**
 * @brief Equilibreuse::on_actionEnregistrer_triggered
 * @details Enregistre dans un fichier .equ les courbes de l'expérience courante.
 */
void Equilibreuse::on_actionEnregistrer_triggered()
{
    QFileDialog fichierDlg(this);
    QString cheminDuFichier;
    fichierDlg.setAcceptMode(QFileDialog::AcceptSave);
    fichierDlg.setWindowTitle("Enregistrement d'une expérience d'équilibrage");
    fichierDlg.setViewMode(QFileDialog::Detail);
    fichierDlg.setOption(QFileDialog::DontResolveSymlinks);
    fichierDlg.setNameFilter("Fichier d'équilibrage (*equ)");
    fichierDlg.setDefaultSuffix("equ");     // ne fonctionnne pas
    if(fichierDlg.exec() && fichierDlg.selectedFiles().size() != 0)
    {
        cheminDuFichier = fichierDlg.selectedFiles()[0];
        QStringList chemin = cheminDuFichier.split(".");
        if(chemin.last() != "equ")
            cheminDuFichier += ".equ";
        if (!cheminDuFichier.isEmpty())
            experience->EnregistrerCourbes(cheminDuFichier);
    }
}

/**
 * @brief Equilibreuse::on_actionImporter_mesures_brutes_triggered
 * @details Ouvre la boite de dialogue pour choisir le nom du fichier contenant
 *          des mesures brutes puis appel les méthodes pour afficher les courbes à l'écran
 */
void Equilibreuse::on_actionImporter_mesures_brutes_triggered()
{
    if(experience != nullptr)
        delete experience;
    experience = new Experience(this);

    QFileDialog fichierDlg(this);
    QString leFichier;
    fichierDlg.setAcceptMode(QFileDialog::AcceptOpen);
    fichierDlg.setWindowTitle("Ouverture d'un fichier de mesures brutes");
    fichierDlg.setViewMode(QFileDialog::Detail);
    fichierDlg.setOption(QFileDialog::DontResolveSymlinks);
    fichierDlg.setNameFilter("fichier MBR (*mbr)");
    if(fichierDlg.exec() && fichierDlg.selectedFiles().size() != 0)
    {
        leFichier = fichierDlg.selectedFiles()[0];
        if (!leFichier.isEmpty())
        {
            experience->LireMesuresBrutes(leFichier);
            experience->InitiliserCourbes(newton,degre);
            MettreAjourCourbe();
            MettreAjourTableauAcquisition();
            MettreAjourVitesseExperience();

            ChangerEtatSysteme(RESTITUTION);
        }
    }
}

/**
 * @brief Equilibreuse::on_actionFormat_PDF_triggered
 * @details : ouvre le fichier qui va recevoir l'expérience au format PDF
 *            pour être complet le rapport doit être complété dans la configuration
 *            de l'expérience.
 */
void Equilibreuse::on_actionFormat_PDF_triggered()
{
    QFileDialog fichierDlg(this);
    QString cheminDuFichier;
    fichierDlg.setAcceptMode(QFileDialog::AcceptSave);
    fichierDlg.setWindowTitle("Création d'un fichier PDF");
    fichierDlg.setViewMode(QFileDialog::Detail);
    fichierDlg.setOption(QFileDialog::DontResolveSymlinks);
    fichierDlg.setNameFilter("fichier PDF (*pdf)");
    fichierDlg.setDefaultSuffix("pdf");
    if(fichierDlg.exec() && fichierDlg.selectedFiles().size() != 0)
    {
        cheminDuFichier = fichierDlg.selectedFiles()[0];
        RapportMesures  rapport(*experience,*chartView,cheminDuFichier);
        rapport.ConstruireRapport();
    }
}

/**
 * @brief Equilibreuse::on_actionFormat_CSV_triggered
 * @details Ouvre la boite de dialogue permettant de choisir le nom du fichier CSV
 *          Lance l"a conversion'exportation des courbes au format CSV en fonction
 *          des options d'affichage, si un fichier à été sélectionné
 */
void Equilibreuse::on_actionFormat_CSV_triggered()
{
    if(experience != nullptr)
    {
        QString nomFichier;
        nomFichier = QFileDialog::getSaveFileName(this,
                                                  tr("Exporter au format CSV"),
                                                  "",
                                                  tr("Fichiers CSV(*.csv)"));
        if(nomFichier != nullptr)
        {
            if (!nomFichier.isEmpty())
            {
                if (QFileInfo(nomFichier).suffix().isEmpty())
                    nomFichier.append(".csv");
                experience->ExporterCSV(nomFichier,newton,degre);
            }
        }
    }
}

/**
 * @brief Equilibreuse::on_actionConfigurer_triggered
 * @details : Ouvre la boîte de configuration d'une expérience
 */
void Equilibreuse::on_actionConfigurer_triggered()
{
    DialogConfigurerExperience dlgConfig(experience,this);
    dlgConfig.exec();
}

/**
 * @brief Equilibreuse::on_actionAffichage_en_degre_toggled
 * @param arg1
 * @details Affiche les courbes en fonction de la position angulaire en degré si arg1 est VRAI,
 *          sinon affiche les courbes en fonction des points du codeur.
 *          Si les courbes peuvent être initialisées, elles s'affiche avec la nouvelle unité
 */
void Equilibreuse::on_actionAffichage_en_degre_toggled(bool arg1)
{
    degre = arg1;
    if(experience != nullptr)
    {
        if(experience->InitiliserCourbes(newton,degre))
            MettreAjourCourbe();
    }
}

/**
 * @brief Equilibreuse::on_actionAffichage_en_Newton_toggled
 * @param arg1
 * @details Autorise l'affichage en Newton si arg1 est VRAI sinon l'affichage
 *          se fera en Volts
 *          Si les courbes peuvent être initialisées, elles s'affiche avec la nouvelle unité
 */
void Equilibreuse::on_actionAffichage_en_Newton_toggled(bool arg1)
{
    newton = arg1;
    if(experience != nullptr)
    {
        if(experience->InitiliserCourbes(newton,degre))
            MettreAjourCourbe();
    }
}

/**
 * @brief Equilibreuse::on_actionParametrer_triggered
 * @details Ouvre la boîte de dialogue de paramétrage pour la création
 *          ou la mise à jour du fichier ini
 *          Attention : une expérience doit être créée pour pouvoir ouvrir la boîte
 */
void Equilibreuse::on_actionParametrer_triggered()
{
    if(experience != nullptr)
    {
        DialogParametrageEquilibreuse dlgParametrage(experience,this);
        dlgParametrage.exec();
    }
}

/* ------- Gestion de l'IHM ----------------- */

/**
 * @brief Equilibreuse::ChangerEtatSysteme
 * @param etat
 * @details Active ou désactive chaque élément de l'interface en fonction
 *          de l'état du système.
 */
void Equilibreuse::ChangerEtatSysteme(Equilibreuse::ETAT_SYSTEME etat)
{
    QPalette palette;
    palette.setColor(QPalette::WindowText,Qt::black);
    labelEtatCapot.setPalette(palette);
    etatDuSysteme = etat;
    switch(etatDuSysteme)
    {
    case REPOS:
        ui->groupBoxVitesse->setVisible(false);
        ui->groupeBoxAcquisition->setDisabled(true);
        ui->groupeBoxConsigneVitesse->setDisabled(true);
        ui->groupeBoxOrigine->setDisabled(true);
        ui->buttonArretMoteur->setDisabled(true);
        ui->buttonLancerMoteur->setEnabled(true);
        ui->actionEnregistrer->setDisabled(true);
        ui->menuExporter->setDisabled(true);
        ui->actionConfigurer->setDisabled(true);
        if(experience == nullptr)
            ui->menuExporter->setEnabled(false);
        labelEtatSysteme.setText("| Repos ");
        break;
    case PRISE_ORIGINE:
        ui->groupeBoxOrigine->setEnabled(true);
        labelEtatSysteme.setText("| Prise de l'origine ");
        break;
    case ATTENTE_CONSIGNE_VITESSE:
        ui->groupeBoxOrigine->setDisabled(true);
        ui->groupeBoxConsigneVitesse->setEnabled(true);
        ui->groupeBoxAcquisition->setDisabled(true);
        ui->buttonLancerMoteur->setEnabled(true);
        ui->buttonLancerAcquisition->setText("Lancer\nAcquisition");
        labelEtatSysteme.setText("| Attente consigne vitesse ");
        break;
    case EN_ROTATION:
        ui->actionEnregistrer->setDisabled(true);
        ui->buttonArretMoteur->setEnabled(true);
        ui->buttonLancerAcquisition->setEnabled(true);
        ui->groupeBoxAcquisition->setEnabled(true);
        ui->groupBoxVitesse->setVisible(false);
        labelEtatSysteme.setText("| En rotation ");
        ui->buttonLancerAcquisition->setText("Lancer\nAcquisition");
        break;
    case SECURITE:
        ui->buttonLancerAcquisition->setDisabled(true);
        ui->buttonArretMoteur->setDisabled(true);
        ui->buttonLancerMoteur->setDisabled(true);
        labelEtatSysteme.setText("| Sécurité ");
        break;
    case EXPERIENCE_FIGEE:
        ui->groupBoxVitesse->setVisible(true);
        ui->actionEnregistrer->setEnabled(true);
        if(experience == nullptr)
            ui->menuExporter->setEnabled(false);
        else
            ui->menuExporter->setEnabled(true);
        ui->buttonLancerAcquisition->setText("Lancer\nAcquisition");
        labelEtatSysteme.setText("| Expérience figée ");
        break;
    case RESTITUTION:
        ui->groupeBoxOrigine->setDisabled(true);
        ui->groupeBoxConsigneVitesse->setDisabled(true);
        ui->buttonLancerAcquisition->setDisabled(true);
        ui->groupeBoxAcquisition->setEnabled(true);
        ui->groupBoxVitesse->setVisible(true);
        if(experience == nullptr)
            ui->menuExporter->setEnabled(false);
        else
            ui->menuExporter->setEnabled(true);
        ui->actionEnregistrer->setEnabled(true);
        ui->actionConfigurer->setEnabled(true);
        labelEtatSysteme.setText("| Mode restitution ");
        break;
    case ACQUISITION:
        labelEtatSysteme.setText("| Acquisition ");
        ui->buttonLancerAcquisition->setText("Figer\nAcquisition");
        break;
    case EN_DEMARRAGE:
        ui->buttonLancerAcquisition->setEnabled(true);
        ui->groupeBoxAcquisition->setEnabled(true);
        ui->buttonArretMoteur->setEnabled(true);
        labelEtatSysteme.setText("| Démarrage du moteur ");
        ui->buttonLancerAcquisition->setText("Lancer\nAcquisition");
        break;
    default:
        labelEtatSysteme.setText("| non défini ");
        break;
    }
}

/**
 * @brief Equilibreuse::MettreAjourCourbe
 * details Dessine les courbes sur l'écran
 */
void Equilibreuse::MettreAjourCourbe()
{
    if(experience != nullptr)
    {
        if(vueGraphique != nullptr)
            delete vueGraphique;
        vueGraphique = new VueGraphique(experience);
        if(chartView != nullptr)
            delete chartView;
        if(degre)
            chartView = new QChartView(vueGraphique->DessinerCourbes(360,degre,newton));
        else
            chartView = new QChartView(vueGraphique->DessinerCourbes(NB_POINTS_CODEUR * NB_PERIODE,degre,newton));

        layout->addWidget(chartView);
    }
}

/**
 * @brief Equilibreuse::MettreAjourTableauAcquisition
 * @details Affiche le tableau d'acquisition à partir des données de l'expérience.
 *          Elle deux courbes doivent-être initialisée ou chargée.
 */
void Equilibreuse::MettreAjourTableauAcquisition()
{
    if(experience != nullptr)
    {
        QPointF point;
        point = experience->RechercherEffortMax('A');
        ui->tableWidgetAcquisition->item(0,0)->setText(QString::number(point.y(),'f',1));
        ui->tableWidgetAcquisition->item(0,1)->setText(QString::number(point.x(),'f',0));
        point = experience->RechercherEffortMax('O');
        ui->tableWidgetAcquisition->item(1,0)->setText(QString::number(point.y(),'f',1));
        ui->tableWidgetAcquisition->item(1,1)->setText(QString::number(point.x(),'f',0));
        double valeur;
        valeur = experience->RechercherEffortAzero('A');
        ui->tableWidgetAcquisition->item(0,2)->setText(QString::number(valeur,'f',1));
        valeur = experience->RechercherEffortAzero('O');
        ui->tableWidgetAcquisition->item(1,2)->setText(QString::number(valeur,'f',1));
        valeur = experience->RechercherEffortAquatreVingtDix('A');
        ui->tableWidgetAcquisition->item(0,3)->setText(QString::number(valeur,'f',1));
        valeur = experience->RechercherEffortAquatreVingtDix('O');
        ui->tableWidgetAcquisition->item(1,3)->setText(QString::number(valeur,'f',1));
    }
}

/**
 * @brief Equilibreuse::MettreAjourVitesseExperience
 * @details Affiche la vitesse de l'expérience
 */
void Equilibreuse::MettreAjourVitesseExperience()
{
    if(experience != nullptr)
    {
        qint16 vitesseExperience = experience->ObtenirVitesse();
        ui->lineEditVitesseExperience->setText(QString::number(vitesseExperience));
    }
}


/* ----- Gestion des boutons de pilotage de la maquette ----- */



/**
 * @brief Equilibreuse::on_buttonLancerAcquisition_clicked
 * @details Lance l'acquisition des courbes si le système est en mode
 *          EN_ROTATION (cas d'origine),
 *          EXPERIENCE_FIGE (cas de reprise),
 *       ou ACQUISITION (cas pour modifier la vitesse de rotation en cours d'expérience)
 */
void Equilibreuse::on_buttonLancerAcquisition_clicked()
{ 
    switch (etatDuSysteme)
    {
    case REPOS:
    case PRISE_ORIGINE:
    case RESTITUTION:
    case ATTENTE_CONSIGNE_VITESSE:
    case SECURITE:
        break;

    case EN_DEMARRAGE:
    case EN_ROTATION:
    case EXPERIENCE_FIGEE:
        LancerAcquisition();
        ChangerEtatSysteme(ACQUISITION);
        break;

    case ACQUISITION:
        ArreterAcquisition();
        ui->lineEditVitesseExperience->setVisible(true);
        ui->lineEditVitesseExperience->setText(QString::number(vitesse));
        experience->FixerVitesseExperience(vitesse);
        ChangerEtatSysteme(EXPERIENCE_FIGEE);
    }
}

/**
 * @brief Equilibreuse::on_buttonLancerMoteur_clicked
 * @details Lance le moteur ou modifie la consigne de vitesse lorsque le
 *          moteur est lancé. La consigne doit être supérieur ou égal
 *          au minimum à 10%
 *          La mesure de vitesse est également lancé par cette méthode
 *          tant que le moteur tourne.
 */
void Equilibreuse::on_buttonLancerMoteur_clicked()
{
    consigne = ui->lcdNumberPourcentageVitesse->value();
    if(consigne > 9)
    {
        if(etatDuSysteme == ATTENTE_CONSIGNE_VITESSE || etatDuSysteme == EXPERIENCE_FIGEE)
        {
            ui->buttonLancerMoteur->setText("Appliquer \n nouvelle consigne");
            if(leMoteur == nullptr)
                leMoteur = new Moteur(*laCarte,BROCHE_MOTEUR,3.3);
            if(leCodeur == nullptr)
            {
                leCodeur = new Codeur(*laCarte,this);
                connect(leCodeur,&Codeur::VitesseChange,this,&Equilibreuse::on_CodeurVitesseChange);
                leCodeur->LancerMesureVitesse();
            }
            if(etatDuSysteme == ATTENTE_CONSIGNE_VITESSE)
                ChangerEtatSysteme( EN_DEMARRAGE);
        }
        if(etatDuSysteme == EN_DEMARRAGE || etatDuSysteme == EN_ROTATION || etatDuSysteme == ACQUISITION)
            leMoteur->FixerConsigneVitesse(consigne);
    }
    else
    {
        QMessageBox msg;
        msg.setText("Veuillez fixer une consigne de vitesse \n au moins supérieur à 10%");
        msg.exec();
    }
}

/**
 * @brief Equilibreuse::on_buttonArretMoteur_clicked
 * @details Arrête la rotation du moteur et la mesure de vitesse par la même occasion
 *          Les acquisitions sont également arrêtées.
 */
void Equilibreuse::on_buttonArretMoteur_clicked()
{
    consigne = 0 ;
    if(leMoteur != nullptr)
        leMoteur->FixerConsigneVitesse(consigne);
    ui->lcdNumberPourcentageVitesse->display(consigne);
    ui->buttonFixerConsigneVitesse->setValue(0);
    ui->lineEditVitesseMoteur->setText("0");
    ui->buttonLancerMoteur->setText("Lancer Moteur");
    ui->buttonArretMoteur->setDisabled(true);
    ui->groupeBoxAcquisition->setDisabled(true);
    if(leCodeur != nullptr)
    {
        leCodeur->ArreterMessureVitesse();
        delete leCodeur;
        leCodeur = nullptr;
    }
    ChangerEtatSysteme( ATTENTE_CONSIGNE_VITESSE);
    ArreterAcquisition();
}

/**
 * @brief Equilibreuse::on_buttonFixerConsigneVitesse_valueChanged
 * @param value
 * @details Slot pour le changement de couleur le réglage de la consigne de vitesse
 *          BLANC la vitesse doit être augmentée pour faire de bonnes acquisitions
 *          VERT l'acquisition peut se faire dans de bonnes conditions
 *          ROUGE le moteur tourne vite Attention aux jauges de contrainte..
 */
void Equilibreuse::on_buttonFixerConsigneVitesse_valueChanged(int value)
{
    if(value >= LIMITE_VITESSE_BASSE && value <= LIMITE_VITESSE_HAUTE)
    {
        ui->lcdNumberPourcentageVitesse->setPalette(Qt::green);
        ui->buttonFixerConsigneVitesse->setPalette(Qt::green);
    }
    else if (value > LIMITE_VITESSE_HAUTE)
    {
        ui->lcdNumberPourcentageVitesse->setPalette(Qt::red);
        ui->buttonFixerConsigneVitesse->setPalette(Qt::red);
    }
    else
    {
        ui->lcdNumberPourcentageVitesse->setPalette(Qt::white);
        ui->buttonFixerConsigneVitesse->setPalette(Qt::white);
    }
    consigne = value;
}

/* ----- Méthode de gestion de l'application ----- */

/**
 * @brief Equilibreuse::LancerAcquisition
 * @details Créé l'instance pour les jauges de contrainte
 *          Lance la récupération des acquisition périodique
 */
void Equilibreuse::LancerAcquisition()
{
    if(lesJauges == nullptr)
    {
        lesJauges = new JaugesDeContrainte(*laCarte,this);
    }
    if(timerAcquisition == nullptr)
    {
        timerAcquisition = new QTimer(this);
        connect(timerAcquisition,&QTimer::timeout,this,&Equilibreuse::on_TimeOutAcquisitionMesures);
    }
    timerAcquisition->start(TEMPO_ACQUISITION);
}

/**
 * @brief Equilibreuse::ArreterAcquisition
 * @details Arrête l'acquisition des forces exercée sur les paliers.
 */
void Equilibreuse::ArreterAcquisition()
{
    if(timerAcquisition != nullptr)
    {
        timerAcquisition->stop();
        disconnect(timerAcquisition,&QTimer::timeout,this,&Equilibreuse::on_TimeOutAcquisitionMesures);
        delete timerAcquisition;
        timerAcquisition = nullptr;
    }
    if(lesJauges !=nullptr)
    {
        delete lesJauges;
        lesJauges = nullptr;
    }
}

/**
 * @brief Equilibreuse::on_TimeOutAcquisitionMesures
 * @details Assure la capture périodique des données brutes en provenance de la carte d'acquisition.
 *          Prépare les courbes pour l'affichage en fonction des options d'affichage
 *          Met à jour le tableau récapitulatif
 *          Dessine les courbe en fonction des options d'affichage
 */
void Equilibreuse::on_TimeOutAcquisitionMesures()
{
    int nbVal;
    timerAcquisition->stop();
    double *mesuresBrutes = lesJauges->ObtenirMesuresBrutes(nbVal);

    if(mesuresBrutes != nullptr)
    {
        experience->PreparerCourbes(mesuresBrutes,nbVal,newton,degre);
        MettreAjourTableauAcquisition();
        if(degre)
            vueGraphique->DessinerCourbes(360,degre,newton);
        else
            vueGraphique->DessinerCourbes(NB_POINTS_CODEUR * NB_PERIODE,degre,newton);
    }
    timerAcquisition->start(TEMPO_ACQUISITION);
}

/**
 * @brief Equilibreuse::on_CodeurVitesseChange
 * @param _vitesse
 * @details Slot mis à jour par un changement de la vitesse du moteur
 *          affiche cette nouvelle vitesse sur l'écran
 */
void Equilibreuse::on_CodeurVitesseChange(int _vitesse)
{
    vitesse = _vitesse;
    if(vitesse >0)
    {
        ui->lineEditVitesseMoteur->setText(QString::number(vitesse));
        if(vitesse/static_cast<float>(VITESSE_MOTEUR_MAXI)*100 >= consigne && etatDuSysteme == EN_DEMARRAGE)
            ChangerEtatSysteme(EN_ROTATION);
    }
}







