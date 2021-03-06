/*
 * Copyright 2016,2017 Marcus Soll
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "simpleinterface.h"

#include <QtCore/QtMath>

SimpleInterface::SimpleInterface(QObject *parent) :
    QObject(parent),
    _count(0)
{
    recount();
}

int SimpleInterface::count()
{
    return _count;
}

bool SimpleInterface::clearAllVocabulary()
{
    database.transaction();

    QString s = "DELETE FROM vocabulary";
    QSqlQuery q(database);

    if(!q.exec(s))
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return false;
    }

    database.commit();
    _count = 0;
    emit countChanged(_count);
    return true;
}

void SimpleInterface::recount()
{
    _count = 0;
    QString s = "SELECT count(*) FROM vocabulary";
    QSqlQuery q(database);

    if(!q.exec(s))
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return;
    }
    _count = q.value(0).toInt();
    emit countChanged(_count);
}

bool SimpleInterface::addVocabulary(QString word, QString translation, int language)
{
    database.transaction();
    qint64 date = QDate::currentDate().toJulianDay();
    QString s = "INSERT INTO vocabulary (word, translation, priority, creation, modification, language, number_asked, number_correct) VALUES (:word, :translation, 100, :creation, :modification, :language, 0, 0)";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":word", word.simplified());
    q.bindValue(":translation", translation.simplified());
    q.bindValue(":creation", date);
    q.bindValue(":modification", date);
    q.bindValue(":language", language);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return false;
    }

    database.commit();
    _count += 1;
    emit countChanged(_count);
    return true;
}

bool SimpleInterface::removeVocabulary(int id)
{
    database.transaction();

    QString s = "DELETE FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return false;
    }

    database.commit();
    _count -= 1;
    emit countChanged(_count);
    return true;
}

bool SimpleInterface::editVocabulary(int id, QString new_word, QString translation, int priority, int language)
{
    priority = qBound(1, priority, 100);
    new_word = new_word.simplified();
    translation = translation.simplified();

    // Update entry
    database.transaction();

    QSqlQuery q(database);
    QString s = "UPDATE vocabulary SET word=:w, translation=:t, priority=:p, modification=:m, language=:l WHERE rowid=:id";
    q.prepare(s);
    q.bindValue(":w", new_word);
    q.bindValue(":t", translation);
    q.bindValue(":p", priority);
    q.bindValue(":m", QDate::currentDate().toJulianDay());
    q.bindValue(":l", language);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return false;
    }

    database.commit();
    return true;
}

bool SimpleInterface::setPriority(int id, int priority)
{
    QString s = "UPDATE vocabulary SET priority=:p WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":p", priority);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return false;
    }

    return true;
}
QList<int> SimpleInterface::getAllWords(sortcriterium c)
{
    QString s = "SELECT rowid FROM vocabulary";
    QSqlQuery q(database);

    append_sorting_criterium(s, c);

    q.prepare(s);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return QList<int>();
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return QList<int>();
    }

    QList<int> vl;
    while(q.next())
    {
        vl.append(q.value(0).toInt());
    }
    return vl;

}

QString SimpleInterface::getWord(int id)
{
    QString s = "SELECT word FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return "";
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return "";
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return "";
    }
    return q.value(0).toString();
}

QString SimpleInterface::getTranslationOfWord(int id)
{
    QString s = "SELECT translation FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return "";
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return "";
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return "";
    }
    return q.value(0).toString();
}

int SimpleInterface::getPriorityOfWord(int id)
{
    QString s = "SELECT priority FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return 100;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return 100;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return 100;
    }
    return q.value(0).toInt();
}

QDate SimpleInterface::getCreationDate(int id)
{
    QString s = "SELECT creation FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return QDate::fromJulianDay(1);
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return QDate::fromJulianDay(1);
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return QDate::fromJulianDay(1);
    }
    return QDate::fromJulianDay(q.value(0).toLongLong());
}

QDate SimpleInterface::getModificationDate(int id)
{
    QString s = "SELECT modification FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return QDate::fromJulianDay(1);
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return QDate::fromJulianDay(1);
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return QDate::fromJulianDay(1);
    }
    return QDate::fromJulianDay(q.value(0).toLongLong());
}

int SimpleInterface::getLanguageId(int id)
{
    QString s = "SELECT language FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return 1;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return 1;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return 1;
    }
    return q.value(0).toInt();
}

int SimpleInterface::getNumberAsked(int id)
{
    QString s = "SELECT number_asked FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return 0;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return 0;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return 0;
    }
    return q.value(0).toInt();
}

int SimpleInterface::getNumberCorrect(int id)
{
    QString s = "SELECT number_correct FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return 0;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return 0;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return 0;
    }
    return q.value(0).toInt();
}


bool SimpleInterface::removeBatchVocabulary(QList<int> ids)
{
    QString s = "DELETE FROM vocabulary WHERE rowid=:id";
    QSqlQuery q(database);

    database.transaction();

    for(QList<int>::const_iterator i = ids.constBegin(); i != ids.constEnd(); ++i)
    {
        q.prepare(s);
        q.bindValue(":id", *i);
        if(!q.exec())
        {
            QString error = s;
            error.append(": ").append(q.lastError().text());
            WARNING(error);
            database.rollback();
            return false;
        }
    }

    database.commit();
    return true;
}

QList<QString> SimpleInterface::getBatchWord(QList<int> ids)
{
    QList<QString> result;

    database.transaction();
    for(QList<int>::const_iterator i = ids.constBegin(); i != ids.constEnd(); ++i)
    {
        result.push_back(getWord(*i));
    }
    database.commit();
    return result;
}

QList<QString> SimpleInterface::getBatchTranslationOfWord(QList<int> ids)
{
    QList<QString> result;
    database.transaction();
    for(QList<int>::const_iterator i = ids.constBegin(); i != ids.constEnd(); ++i)
    {
        result.push_back(getTranslationOfWord(*i));
    }
    database.commit();
    return result;
}

QList<int> SimpleInterface::getBatchPriorityOfWord(QList<int> ids)
{
    QList<int> result;
    database.transaction();
    for(QList<int>::const_iterator i = ids.constBegin(); i != ids.constEnd(); ++i)
    {
        result.push_back(getPriorityOfWord(*i));
    }
    database.commit();
    return result;
}

bool SimpleInterface::resetTestCountsAll()
{
    QString s = "UPDATE vocabulary SET number_asked=0, number_correct=0";
    QSqlQuery q(database);

    q.prepare(s);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return false;
    }

    return true;
}

bool SimpleInterface::resetTestCounts(int id)
{
    QString s = "UPDATE vocabulary SET number_asked=0, number_correct=0 WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return false;
    }

    return true;
}

bool SimpleInterface::resetPriorityAll()
{
    QString s = "UPDATE vocabulary SET priority=100";
    QSqlQuery q(database);

    q.prepare(s);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return false;
    }

    return true;
}

float SimpleInterface::getOverallPercentageCorrect()
{
    QString s = "SELECT AVG(CAST(number_correct AS REAL)/CAST(number_asked AS REAL)) FROM vocabulary";
    QSqlQuery q(database);

    q.prepare(s);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return 0.0f;
    }
        if(!q.next())
    {
        QString error = s;
        error.append(" - no average found: ").append(q.lastError().text());
        WARNING(error);
        return 0.0f;
    }

    return q.value(0).toFloat();
}

QVariantList SimpleInterface::getAllLanguages()
{
    QString s = "SELECT rowid FROM language ORDER BY language ASC";
    QSqlQuery q(database);

    q.prepare(s);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return QVariantList();
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return QVariantList();
    }

    QVariantList vl;
    while(q.next())
    {
        vl.append(q.value(0).toInt());
    }
    return vl;
}

int SimpleInterface::addLanguage(QString language)
{
    database.transaction();
    QString s = "INSERT INTO language (language) VALUES (:language)";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":language", language);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return -1;
    }
    database.commit();

    s = "SELECT last_insert_rowid()";

    if(!q.exec(s))
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return -1;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return -1;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return -1;
    }
    return q.value(0).toInt();
}

bool SimpleInterface::removeLanguage(int id)
{
    if(countVocabularyWithLanguage(id) != 0)
    {
        WARNING("Can not remove a language with vocabulary in it");
        return false;
    }

    database.transaction();

    QString s = "DELETE FROM language WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return false;
    }

    database.commit();
    return true;
}

QString SimpleInterface::getLanguageName(int id)
{
    QString s = "SELECT language FROM language WHERE rowid=:id";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return "";
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return "";
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return "";
    }
    return q.value(0).toString();
}

bool SimpleInterface::renameLanguage(int id, QString name)
{
    name = name.simplified();

    // Update entry
    database.transaction();

    QSqlQuery q(database);
    QString s = "UPDATE language SET language=:l WHERE rowid=:id";
    q.prepare(s);
    q.bindValue(":l", name);
    q.bindValue(":id", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        database.rollback();
        return false;
    }

    database.commit();
    return true;
}

QVariantList SimpleInterface::getVocabularyByLanguage(int id, sortcriterium c)
{
    QString s = "SELECT rowid FROM vocabulary WHERE language=:language";
    QSqlQuery q(database);

    append_sorting_criterium(s, c);

    q.prepare(s);
    q.bindValue(":language", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return QVariantList();
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return QVariantList();
    }

    QVariantList vl;
    while(q.next())
    {
        vl.append(q.value(0).toInt());
    }
    return vl;
}

int SimpleInterface::countVocabularyWithLanguage(int id)
{
    QString s = "SELECT count(*) FROM vocabulary WHERE language=:language";
    QSqlQuery q(database);

    q.prepare(s);
    q.bindValue(":language", id);

    if(!q.exec())
    {
        QString error = s;
        error.append(": ").append(q.lastError().text());
        WARNING(error);
        return -1;
    }
    if(!q.isSelect())
    {
        QString error = s;
        error.append(": No select");
        WARNING(error);
        return -1;
    }
    if(!q.next())
    {
        QString error = s;
        error.append(" - No entry found: ").append(q.lastError().text());
        WARNING(error);
        return -1;
    }
    return q.value(0).toInt();
}

bool SimpleInterface::moveToLanguage(int lid, QVariantList v_list)
{
    QString s = "UPDATE vocabulary SET language=:language, modification=:modification WHERE rowid=:id";
    qint64 date = QDate::currentDate().toJulianDay();
    QSqlQuery q(database);

    database.transaction();

    for(QVariantList::const_iterator i = v_list.constBegin(); i != v_list.constEnd(); ++i)
    {
        if(!(*i).canConvert<int>())
        {
            WARNING(QString("Can not convert %1 to int").arg((*i).typeName()));
            continue;
        }
        q.prepare(s);
        q.bindValue(":language", lid);
        q.bindValue(":modification", date);
        q.bindValue(":id", (*i).toInt());
        if(!q.exec())
        {
            QString error = s;
            error.append(": ").append(q.lastError().text());
            WARNING(error);
            database.rollback();
            return false;
        }
    }

    database.commit();
    return true;
}

void SimpleInterface::append_sorting_criterium(QString &q, const sortcriterium &c)
{
    switch(c)
    {
    case NO_SORT:
        break;
    case ALPHABETICAL_WORD:
        q.append(" ORDER BY word COLLATE NOCASE ASC");
        break;
    case ALPHABETICAL_TRANSLATION:
        q.append(" ORDER BY translation COLLATE NOCASE ASC");
        break;
    case PRIORITY_HIGHEST:
        q.append(" ORDER BY priority DESC");
        break;
    case PRIORITY_LOWEST:
        q.append(" ORDER BY priority ASC");
        break;
    case CREATION_NEWEST:
        q.append(" ORDER BY creation DESC");
        break;
    case CREATION_OLDEST:
        q.append(" ORDER BY creation ASC");
        break;
    case MODIFICATION_NEWEST:
        q.append(" ORDER BY modification DESC");
        break;
    case MODIFICATION_OLDEST:
        q.append(" ORDER BY modification ASC");
        break;
    case NUMBER_ASKED_HIGHEST:
        q.append(" ORDER BY number_asked DESC");
        break;
    case NUMBER_ASKED_LOWEST:
        q.append(" ORDER BY number_asked ASC");
        break;
    case NUMBER_CORRECT_HIGHEST:
        q.append(" ORDER BY number_correct DESC");
        break;
    case NUMBER_CORRECT_LOWEST:
        q.append(" ORDER BY number_correct ASC");
        break;
    case PERCENT_CORRECT_HIGHEST:
        q.append(" ORDER BY CAST(number_correct AS REAL)/CAST(number_asked AS REAL) DESC");
        break;
    case PERCENT_CORRECT_LOWEST:
        q.append(" ORDER BY CAST(number_correct AS REAL)/CAST(number_asked AS REAL) ASC");
        break;
    default:
        WARNING("Unknown sort criterium" << c);
        break;
    }
}
