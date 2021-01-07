/**
 * @file experience.cpp
 * @date Création 19 janvier 2018
 * @date Mise à jour 5 janvier 2021
 * @version USB-202 1.0
 * @author Philippe CRUCHET
 * @details Classe Experience : Gestion des données
 */

#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QDataStream>
#include <QPointF>


#include "experience.h"
#include "constante.h"

/**
 * @brief Experience::Experience
 * @param parent
 * @details Constructeur de la classe Initialise les attribut de la classe
 *          Lance la lecture du fichier ini de l'application
 */
Experience::Experience(QObject *parent) : QObject(parent)
  , vitesseMaxi(0)
  , configExperience(false)
  , nbEchantillons(0)
  , origine(0)
  , vitesse(0)
  , mesuresBrutes(nullptr)

{
    LireFichierIni();
}

/**
 * @brief Experience::LireFichierIni
 * @details Effectue la lecture du fichier ini de l'application,
 *          si celui-ci n'existe pas un fichier est créé avec les valeurs par défaut
 */
void Experience::LireFichierIni()
{
    QString nomFichierIni = "equilibreuse.ini";
    QFileInfo fichierIni(nomFichierIni);
    if (fichierIni.exists() && fichierIni.isFile())
    {
        QSettings param(nomFichierIni,QSettings::IniFormat);
        paliers[PALIER_A].jauge.modele = param.value("JaugeA/modele").toString();
        paliers[PALIER_A].jauge.numeroDeSerie = param.value("JaugeA/serie").toString();
        paliers[PALIER_A].jauge.date = param.value("JaugeA/date").toString();
        paliers[PALIER_A].jauge.capacite = param.value("JaugeA/capacite").toDouble();
        paliers[PALIER_A].jauge.sensibilite = param.value("JaugeA/sensibilite").toDouble();

        paliers[PALIER_O].jauge.modele = param.value("JaugeO/modele").toString();
        paliers[PALIER_O].jauge.numeroDeSerie = param.value("JaugeO/serie").toString();
        paliers[PALIER_O].jauge.date = param.value("JaugeO/date").toString();
        paliers[PALIER_O].jauge.capacite = param.value("JaugeO/capacite").toDouble();
        paliers[PALIER_O].jauge.sensibilite = param.value("JaugeO/sensibilite").toDouble();
    }
    else
    {   //  si le fichier n'existe pas, création d'un fichier par défaut
        paliers[PALIER_A].jauge.modele = "MB-25";
        paliers[PALIER_A].jauge.numeroDeSerie = "C30629";
        paliers[PALIER_A].jauge.date = "09-22-1998";
        paliers[PALIER_A].jauge.capacite = 25;
        paliers[PALIER_A].jauge.sensibilite = 3.348;

        paliers[PALIER_O].jauge.modele = "MB-25";
        paliers[PALIER_O].jauge.numeroDeSerie = "C30637";
        paliers[PALIER_O].jauge.date = "10-05-1998";
        paliers[PALIER_O].jauge.capacite = 25;
        paliers[PALIER_O].jauge.sensibilite = 3.328;

        EnregistreFichierIni(paliers);
    }
}

/**
 * @brief Experience::EnregistreFichierIni
 * @param _paliers
 * @details Enregistre le fichier ini sur le disque
 */
void Experience::EnregistreFichierIni(const typePalier _paliers[])
{
    QString nomFichierIni = "equilibreuse.ini";
    QFileInfo fichierIni(nomFichierIni);
    QSettings param(nomFichierIni,QSettings::IniFormat);
    param.beginGroup("JaugeA");
    param.setValue("modele",_paliers[PALIER_A].jauge.modele);
    param.setValue("serie",_paliers[PALIER_A].jauge.numeroDeSerie);
    param.setValue("date",_paliers[PALIER_A].jauge.date);
    param.setValue("capacite",_paliers[PALIER_A].jauge.capacite);
    param.setValue("sensibilite",_paliers[PALIER_A].jauge.sensibilite);
    param.endGroup();

    param.beginGroup("JaugeO");
    param.setValue("modele",paliers[PALIER_O].jauge.modele);
    param.setValue("serie",paliers[PALIER_O].jauge.numeroDeSerie);
    param.setValue("date",paliers[PALIER_O].jauge.date);
    param.setValue("capacite",paliers[PALIER_O].jauge.capacite);
    param.setValue("sensibilite",paliers[PALIER_O].jauge.sensibilite);
    param.endGroup();
}


/* ---  Gestion des différents formats de fichier --- */

/**
 * @brief Experience::LireMesuresBrutes
 * @param _nomFichier
 * @details Lit un fichier de mesures brutes, le tableau de mesures est alloué
 *          dynamiquement en fonction du nombre total d'échantillons
 *          Le fichier contient :
 *              - le nombre total d'échantillons
 *              - La vitesse de l'expérence
 *              - la valeur du décalage à l'origine
 *              - l'ensemble des mesures brutes
 */
void Experience::LireMesuresBrutes(QString &_nomFichier)
{
    QFile fichierBrute(_nomFichier);
    if (!fichierBrute.open(QIODevice::ReadOnly))
        qDebug() << "Le Fichier " << _nomFichier << " ne peut pas s'ouvrir";
    else
    {
        QDataStream flux(&fichierBrute);
        flux >> nbEchantillons;
        flux >> vitesse;
        flux >> origine ;

        if (mesuresBrutes != nullptr)
        {
            delete[] mesuresBrutes;
        }
        mesuresBrutes = new double[nbEchantillons];

        for(quint32 i = 0 ; i < nbEchantillons ; i++)
            flux >> mesuresBrutes[i] ;

        fichierBrute.close();
    }
}

/**
 * @brief Experience::EnregistrerCourbes
 * @param nomFichier
 * @details Enregistre les deux courbes dans un fichier .equ
 *          le fichier contient : - la vitesse de l'expérience
 *                                - les nom et prénom de l'étudiants, sa classe et son établissement
 *                                - la date et le commentaire de l'expérience
 *                                - les caractéristiques des jauges de contraintre
 *                                - la liste des points de la première courbe (A)
 *                                - la liste des points de la deuxième courbe (O)
 */
void Experience::EnregistrerCourbes(QString &nomFichier)
{
    QFile file(nomFichier);
    bool open = file.open(QIODevice::WriteOnly);
    if (!open)
    {
        qDebug() << "Le Fichier ne peut pas s'ouvrir";
    }
    else
    {
        QDataStream flux(&file);
        flux << vitesse;
        flux << nomEtudiant;
        flux << prenomEtudiant;
        flux << classe;
        flux << etablissement;
        flux << date;
        flux << commentaire;
        flux << paliers[PALIER_A].jauge.modele;            //palier A
        flux << paliers[PALIER_A].jauge.numeroDeSerie;
        flux << paliers[PALIER_O].jauge.modele;            //palier O
        flux << paliers[PALIER_O].jauge.numeroDeSerie;
        flux << courbeA ;
        flux << courbeO ;
        file.close();
    }
}

/**
 * @brief Experience::LireCourbes
 * @param nomFichier
 * @details Lit un fichier .equ
 */
void Experience::LireCourbes(QString &nomFichier)
{
    QFile file(nomFichier);
    bool open = file.open(QIODevice::ReadOnly);
    if (!open)
        qDebug() << "Le Fichier " << nomFichier << " ne peut pas s'ouvrir";
    else
    {
        QDataStream flux(&file);
        flux >> vitesse ;
        flux >> nomEtudiant;
        flux >> prenomEtudiant;
        flux >> classe;
        flux >> etablissement;
        flux >> date;
        flux >> commentaire;
        flux >> paliers[PALIER_A].jauge.modele;
        flux >> paliers[PALIER_A].jauge.numeroDeSerie;
        flux >> paliers[PALIER_O].jauge.modele;
        flux >> paliers[PALIER_O].jauge.numeroDeSerie;
        flux >> courbeA;
        flux >> courbeO;

        file.close();
    }
}

/**
 * @brief Experience::ExporterCSV
 * @param nomFichier
 * @param _newton
 * @param _degre
 * @details Exporte les courbes au format CSV chaque colonne est séparer par un ';'
 *          Le fichier comporte 200 points de mesure 1/5 parmis les 1000 points du
 *          codeur.
 */
void Experience::ExporterCSV(QString &nomFichier, bool _newton, bool _degre)
{
    QFile file(nomFichier);

    bool open = file.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!open)
         qDebug() << "Le Fichier ne peut pas s'ouvrir";
    else
    {
        QTextStream flux(&file);
        flux.setLocale(QLocale::French); // pour transformer les points en virgules dans csv
        flux.setCodec("UTF-8");          // pour les caractères accentués + QStringLiteral

        flux << "Equilbreuse version 2018;" << QStringLiteral(";Date de l'expérience;");
        flux << date.toString(Qt::SystemLocaleLongDate) <<Qt::endl <<Qt::endl;
        QString vitesseString = QString::number(vitesse);  // pour la lecture sous windows !!!
        flux << "Vitesse de rotation;" << vitesseString << "; trs/min" << Qt::endl <<Qt::endl;
        flux << ";" <<"Capteur A;" << "Capteur O" << Qt::endl;
        flux <<"type de capteurs;" << paliers[PALIER_A].jauge.modele << ";" ;
        flux << paliers[PALIER_O].jauge.modele << Qt::endl;
        flux << QStringLiteral("Référence;") << paliers[PALIER_A].jauge.numeroDeSerie;
        flux << ";" << paliers[PALIER_O].jauge.numeroDeSerie << Qt::endl;
        flux << Qt::endl ;
        if(_degre)
            flux <<QStringLiteral("Angle en Degré;");
        else
            flux << "Points du codeur;";

        if(_newton)
            flux << "Courbe A en Newton;" << "Courbe O en Newton;" << Qt::endl;
        else
            flux << "Courbe A en Volt;" << "Courbe O en Volt;" << Qt::endl;

        for(int i=0 ; i < NB_ACQUISITIONS_PAR_PERIODE ; i +=5)
        {
            flux << courbeA.at(i).x() << ";" << courbeA.at(i).y() << ";" << courbeO.at(i).y() << Qt::endl;
        }
        file.close();
    }
}

/* ------- Mise en forme des courbes ------ */

/**
 * @brief Experience::InitiliserCourbes
 * @param _newton
 * @param _degre
 * @return
 * @deprecated Utilisation NI6211 présente pour garder la compatibilité avec les anciens fichiers
 * @details Initilise les tableaux de courbes en fonction des mesures brutes en fonction des options d'affichage
 *          prends en compte les options d'affichage degré et Newton
 *              - degre VRAI affichage que d'une alternance
 *              - newton VRAI affichage avec étalonnage 5kg = 5V à la sortie du conditionneur
 *          l'organisation des mesures brutes est :
 *              - 2000 points pour la courbeA
 *              - 2000 points pour la courbeO
 */
bool Experience::InitiliserCourbes(bool _newton, bool _degre)
{
    bool retour = false;
    if(mesuresBrutes != nullptr)
    {
        int indiceFin = nbEchantillons / 2 ;
        int indiceA = 0;
        int indiceO = nbEchantillons /2 ;
        int i;

        double tensionCalibrageA = 1;
        double tensionCalibrageO = 1;

        if(_newton) // étalonnage 5kg => 5V à la sortie du conditionneur
        {
            tensionCalibrageA = 9.81 ;
            tensionCalibrageO = 9.81 ;
        }

        courbeA.clear();
        courbeO.clear();
        PointDeCourbe unPoint;
        for(i = 0 ; i < (_degre?NB_POINTS_CODEUR:indiceFin) ; i++)
        {
            if(_degre)
                unPoint.setX(i*360/static_cast<float>(NB_POINTS_CODEUR));
            else
                unPoint.setX(i);

            unPoint.setY(mesuresBrutes[origine + indiceA++]*tensionCalibrageA);
            courbeA.append(unPoint);
            unPoint.setY(mesuresBrutes[origine + indiceO++]*tensionCalibrageO);
            courbeO.append(unPoint);
        }
        if(!courbeA.isEmpty() && !courbeO.isEmpty())
            retour = true;
    }
    return retour;
}

/**
 * @brief Experience::PreparerCourbes
 * @param _mesuresBrutes
 * @param _nbEchantillons
 * @param _newton
 * @param _degre
 * @return
 * @details Initilise les tableaux de courbes en fonction des mesures brutes en fonction des options d'affichage
 *          prends en compte les options d'affichage degré et Newton. le décalage à l'origine est pris en compte si
 *          la courbe est affiché sur une alternance en choisissant l'option degré = VRAI
 *              - degre VRAI affichage que d'une alternance
 *              - newton VRAI affichage avec étalonnage 5kg = 5V à la sortie du conditionneur
 *          l'organisation des mesures brutes est un point de la courbeA alternés avec ceux de la courbeO
 *          Utilisation possible uniquement avec des mesures en provenance de la carte USB 202
 */
bool Experience::PreparerCourbes(double *_mesuresBrutes, int _nbEchantillons, bool _newton,bool _degre) // version MCC USB202
{
    bool retour = false;
    int indiceFin ;
    int decalageA = 0 ;
    int decalageO = 0 ;

    courbeA.clear();
    courbeO.clear();

    double tensionCalibrageA = 1;
    double tensionCalibrageO = 1;

    if(_newton) // étalonnage 5kg => 5V à la sortie du conditionneur
    {
        tensionCalibrageA = 9.81 ;
        tensionCalibrageO = 9.81 ;
    }

    if(_degre)
    {
        indiceFin = NB_POINTS_CODEUR;
        decalageA = origine;
        decalageO = origine;
    }
    else
    {
        indiceFin =_nbEchantillons;
    }

    nbEchantillons = _nbEchantillons * NB_CANAUX;
    PointDeCourbe unPoint;
    for (qint32 i=0;i<indiceFin; i+=NB_CANAUX)
    {
        if(_degre)
            unPoint.setX(i*360/static_cast<float>(NB_POINTS_CODEUR));
        else
            unPoint.setX(i);
        unPoint.setY(_mesuresBrutes[decalageA + i]*tensionCalibrageA);
        courbeA.append(unPoint);
        unPoint.setY(_mesuresBrutes[decalageO + i+1]*tensionCalibrageO);
        courbeO.append(unPoint);
    }
    if(!courbeA.isEmpty() && !courbeO.isEmpty())
        retour = true;

    return retour;
}

/* ---- Fonction de recherche des valeurs caractéristiques dans les courbes ------ */

/**
 * @brief Experience::RechercherEffortMax
 * @param courbe définit sur qu'elle courbe la recherche est effectuée 'A' ou 'O'
 * @return Le point possedant la valeur de l'effort le plus grand
 * @details Recherche le point dont l'amplitude est la plus grande sur l'une des 2 courbes.
 */
QPointF Experience::RechercherEffortMax(QChar courbe)
{
    QList<PointDeCourbe> *laCourbe = &courbeO;
    if(courbe =='A')
        laCourbe = &courbeA;
    QList<PointDeCourbe>::iterator max = std::max_element(laCourbe->begin(), laCourbe->end());
    return *max;
}

/**
 * @brief Experience::RechercherEffortAzero
 * @param courbe définit sur qu'elle courbe la recherche est effectuée 'A' ou 'O'
 * @return la valeur de l'effort pour à l'origine de la courbe
 */
double Experience::RechercherEffortAzero(QChar courbe)
{
    QList<PointDeCourbe> *laCourbe = &courbeO;
    if(courbe =='A')
        laCourbe = &courbeA;
    return laCourbe->at(0).y();
}

/**
 * @brief Experience::RechercherEffortAquatreVingtDix
 * @param courbe définit sur qu'elle courbe la recherche est effectuée 'A' ou 'O'
 * @return la valeur de l'effort à 90°
 */
double Experience::RechercherEffortAquatreVingtDix(QChar courbe)
{
    QList<PointDeCourbe> *laCourbe = &courbeO;
    if(courbe =='A')
        laCourbe = &courbeA;
    return laCourbe->at(250).y();   // 90° * 1000 ptsCodeur / 360°
}

/* ---- Ascesseurs de la classe ---- */

typePalier *Experience::ObtenirCarateristiquesPaliers()
{
    return paliers;
}

QList<QPointF> *Experience::ObtenirCourbeA()
{
    return (QList<QPointF>*) &courbeA;
}

QList<QPointF> *Experience::ObtenirCourbeO()
{
    return (QList<QPointF>*) &courbeO;
}

qint16 Experience::ObtenirVitesse()
{
    return vitesse;
}

void Experience::FixerVitesseExperience(int _vitesse)
{
    vitesse = _vitesse;
}

void Experience::FixerOrigine(quint16 _origine)
{
    origine = _origine;
}

QString Experience::ObtenirEtablissement() const
{
    return etablissement;
}

void Experience::ModifierEtablissement(const QString &value)
{
    etablissement = value;
}

QString Experience::ObtenirNom() const
{
    return nomEtudiant;
}

void Experience::ModifierNom(const QString &value)
{
    nomEtudiant = value;
}

QString Experience::ObtenirPrenom() const
{
    return prenomEtudiant;
}

void Experience::ModifierPrenom(const QString &value)
{
    prenomEtudiant = value;
}

QString Experience::ObtenirClasse() const
{
    return classe;
}

void Experience::ModifierClasse(const QString &value)
{
    classe = value;
}

QString Experience::ObtenirCommentaire() const
{
    return commentaire;
}

void Experience::ModifierCommentaire(const QString &value)
{
    commentaire = value;
}

QString Experience::ObtenirDate() const
{
    return date.toString("dddd d MMMM yyyy");
}

void Experience::ModifierDate(const QDate &value)
{
    date = value;
}
void Experience::ValiderConfig(bool _etat)
{
    configExperience = _etat;
}
