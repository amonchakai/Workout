/*
 * SummaryController.cpp
 *
 *  Created on: 3 mai 2015
 *      Author: pierre
 */

#include "SummaryController.hpp"
#include "Database.hpp"
#include "DataObject.hpp"

#include <bb/cascades/AbstractPane>
#include <bb/cascades/GroupDataModel>

SummaryController::SummaryController(QObject *parent) : QObject(parent), m_ListView(NULL) {

}


void SummaryController::getInfos() {

    // --------------------------------------------------------------------
    // date last exercise

    QDateTime lastWorkout = Database::get()->getLastExerciseDate();
    QDateTime today = QDateTime::currentDateTime();


    if(lastWorkout.toMSecsSinceEpoch() == -210866803200000) {
        m_Date = "You need to do some exercise!";
    } else {
        if(today.date() == lastWorkout.date()) {
            m_Date = tr("Today\n last exercise at: ") + lastWorkout.time().toString("hh:mm");
        } else {
            qint64 diffDate = (today.toMSecsSinceEpoch() / (1000*60*60*24)) - (lastWorkout.toMSecsSinceEpoch() / (1000*60*60*24));
            if(diffDate == 1)
                m_Date = tr(" Yesterday\n last exercise at: ") + lastWorkout.time().toString("hh:mm");
            else
                m_Date = QString::number(diffDate) + tr(" days ago on\n") + lastWorkout.date().toString(Qt::SystemLocaleLongDate) ;
        }
    }

    emit dateChanged();

    QSettings settings("Amonchakai", "Workout");

    // --------------------------------------------------------------------
    // general statistics about the last exercise


    QList<QPair<QString, QList<Set*> > >    exercises_strength = Database::get()->getHistoryStrength(lastWorkout.addDays(-1).toMSecsSinceEpoch());
    QList<QPair<QString, QList<Cardio*> > > exercises_cardio   = Database::get()->getHistoryCardio  (lastWorkout.addDays(-1).toMSecsSinceEpoch());

    m_Stats = QString::number(exercises_strength.length() + exercises_cardio.length()) + tr(" exercises completed\n");

    int nbSets = 0;
    float weight = 0;
    for(int i = 0 ; i < exercises_strength.length() ; ++i) nbSets += exercises_strength.at(i).second.length();
    m_Stats += QString::number(nbSets) + tr(" sets completed\n");

    int nbReps = 0;
    for(int i = 0 ; i < exercises_strength.length() ; ++i) {
        for(int k = 0 ; k < exercises_strength.at(i).second.length() ; ++k) {
            nbReps += exercises_strength.at(i).second.at(k)->getRepetition();
            weight += exercises_strength.at(i).second.at(k)->getWeight();
        }
    }
    m_Stats += QString::number(nbReps) + tr(" reps completed\n");

    if(settings.value("unit", 2).toInt() == 2)
        m_Stats += QString::number(weight) + tr(" Kg total lifted\n");
    else
        m_Stats += QString::number(weight) + tr(" lbs total lifted\n");

    int distance = 0;
    int duration = 0;
    for(int i = 0 ; i < exercises_cardio.length() ; ++i) {
        for(int k = 0 ; k < exercises_cardio.at(i).second.length() ; ++k) {
            distance += exercises_cardio.at(i).second.at(k)->getDistance();
            duration += exercises_cardio.at(i).second.at(k)->getDuration();
        }
    }

    if(settings.value("unit", 2).toInt() == 2)
        m_Stats += QString::number(distance) + tr("m in ");
    else
        m_Stats += QString::number(distance) + tr("ft in ");

    m_Stats += QString::number(duration/60) + tr(" min\n");


    emit statsChanged();



    // --------------------------------------------------------------------
    // detailed information about the last exercise

    if(m_ListView == NULL)
        return;

    // ----------------------------------------------------------------------------------------------
    // get the dataModel of the listview if not already available
    using namespace bb::cascades;


    if(m_ListView == NULL) {
        qWarning() << "did not received the listview. quit.";
        return;
    }

    GroupDataModel* dataModel = dynamic_cast<GroupDataModel*>(m_ListView->dataModel());
    if (dataModel) {
        dataModel->clear();
    } else {
        qDebug() << "create new model";
        dataModel = new GroupDataModel(
                QStringList() << "title"
                              << "text"
                );
        m_ListView->setDataModel(dataModel);
    }

    // ----------------------------------------------------------------------------------------------
    // push data to the view

    bool metric = settings.value("unit", 2).toInt() == 2;

    QList<QObject*> datas;
    for(int i = 0 ; i < exercises_strength.length() ; ++i) {
        for(int k = 0 ; k < exercises_strength.at(i).second.length() ; ++k) {
            Detail *d = new Detail();
            d->setTitle(exercises_strength.at(i).first);
            const Set *set = exercises_strength.at(i).second.at(k);
            if(metric)
                d->setText(tr("SET ") + QString::number(set->getId()) + ": " + QString::number(set->getRepetition()) + " REPS @ " + QString::number(set->getWeight()) + tr("Kg") );
            else
                d->setText(tr("SET ") + QString::number(set->getId()) + ": " + QString::number(set->getRepetition()) + " REPS @ " + QString::number(set->getWeight()) + tr("lbs") );

            datas.push_back(d);
        }
    }

    for(int i = 0 ; i < exercises_cardio.length() ; ++i) {
        for(int k = 0 ; k < exercises_cardio.at(i).second.length() ; ++k) {
            Detail *d = new Detail();
            d->setTitle(exercises_cardio.at(i).first);
            const Cardio *cardio = exercises_cardio.at(i).second.at(k);
            if(metric)
                d->setText(QString::number(cardio->getDistance()) + tr("m in ") + QString::number(cardio->getDuration()/60) + tr("min"));
            else
                d->setText(QString::number(cardio->getDistance()) + tr("ft in ") + QString::number(cardio->getDuration()/60) + tr("min"));

            datas.push_back(d);
        }
    }

    dataModel->insertList(datas);
}


