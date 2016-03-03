/******************************************************************************
 *  Copyright (C) 2013 – 2016 by David Baum <david.baum@naraesk.eu>           *
 *                                                                            *
 *  This library is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as published  *
 *  by the Free Software Foundation; either version 2 of the License or (at   *
 *  your option) any later version.                                           *
 *                                                                            *
 *  This library is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *  Library General Public License for more details.                          *
 *                                                                            *
 *  You should have received a copy of the GNU Lesser General Public License  *
 *  along with this library; see the file COPYING.LIB.                        *
 *  If not, see <http://www.gnu.org/licenses/>.                               *
 *****************************************************************************/

#include "translator.h"
#include "translator_config.h"
#include "glosbe.h"
#include "yandex.h"

#include <KLocalizedString>
#include <QApplication>
#include <QClipboard>

Translator::Translator(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args);
    
    setObjectName(QLatin1String("Translator"));
    reloadConfiguration();
    setHasRunOptions(true);
    setIgnoredTypes(Plasma::RunnerContext::Directory |
                    Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation);
    setSpeed(AbstractRunner::SlowSpeed);
    setPriority(HighestPriority);
    setDefaultSyntax(Plasma::RunnerSyntax(QString::fromLatin1("%1:q:").arg(i18n("<language code>")),i18n("Translates the word(s) :q: into target language")));
    setDefaultSyntax(Plasma::RunnerSyntax(QString::fromLatin1("%1:q:").arg(i18n("<source languagce>-<target languagce>")), i18n("Translates the word(s) :q: from the source into target language")));
}

Translator::~Translator()
{
}

bool Translator::parseTerm(const QString& term, QString& text, QPair<QString, QString> &language)
{
    const int index = term.indexOf(" ");
    if (index == -1) return false;
    text = term.mid(index + 1);
    QString languageTerm = term.left(index);

    if (languageTerm.contains("-")) {
        int index = languageTerm.indexOf("-");
        language.first = languageTerm.left(index);
        language.second = languageTerm.mid(index + 1);

    } else {
        if (m_primary == languageTerm) {
            language.first = m_secondary;
        } else {
            language.first = m_primary;
        }   
        language.second = languageTerm;
    }
    return true;
}

void Translator::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QString text;
    QPair<QString, QString> language;
    
    
    if (!parseTerm(term, text, language)) return;
    if (!context.isValid()) return;
    
    if (text.contains(" ")) {
        QEventLoop loop;
        Yandex yandex(this, context, text, language, m_yandexKey);
        connect(&yandex, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    } else {
        QEventLoop loop;
        Glosbe glosbe(this, context, text, language);
        connect(&glosbe, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }
}

void Translator::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    QApplication::clipboard()->setText(match.text());
}

void Translator::reloadConfiguration()
{
    KConfigGroup grp = config();

    m_primary = grp.readEntry(CONFIG_PRIMARY);
    m_secondary = grp.readEntry(CONFIG_SECONDARY);
    m_yandexKey = grp.readEntry(CONFIG_YANDEX_KEY);
}

#include "moc_translator.cpp"