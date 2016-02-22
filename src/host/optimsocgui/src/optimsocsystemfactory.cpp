/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include "optimsocsystemfactory.h"

#include <QFileInfo>
#include <QProcessEnvironment>
#include <QtDebug>

#include "computetile.h"
#include "memorytile.h"
#include "externaltile.h"
#include "meshnoc.h"

/**
 * Current default system
 * @see setCurrentSystem()
 * @see currentSystem()
 */
OptimsocSystem* OptimsocSystemFactory::s_currentSystem = NULL;
OptimsocSystemFactory* OptimsocSystemFactory::s_instance = NULL;

OptimsocSystemFactory::OptimsocSystemFactory(QObject *parent) : QObject(parent)
{
}

/**
 * Get the only instance of this class
 *
 * All methods of this class are static. You only need to get an instance
 * of this class if you want to connect to one of the signals, e.g.
 * currentSystemChanged().
 *
 * @return
 */
OptimsocSystemFactory* OptimsocSystemFactory::instance()
{
    if (!OptimsocSystemFactory::s_instance) {
        OptimsocSystemFactory::s_instance = new OptimsocSystemFactory();
    }
    return OptimsocSystemFactory::s_instance;
}

/**
 * Create a OptimsocSystem object for a system with the given @p systemId
 */
OptimsocSystem* OptimsocSystemFactory::createSystemFromId(int systemId)
{
    QString systemDescriptionsDir = OptimsocSystemFactory::getSysdescDir();

    // see if we have a suitable system description available
    QString sysDescFile = QString("%1/%2.xml")
                              .arg(systemDescriptionsDir)
                              .arg(systemId, 4, 16, QLatin1Char('0'));
    if (!QFileInfo(sysDescFile).exists()) {
        qWarning("No system description available for ID 0x%04x. The XML "
                 "description file %s could not be read.", systemId,
                 sysDescFile.toLatin1().data());
        return NULL;
    }

    return new OptimsocSystem(sysDescFile);
}

/**
 * Set the current default system.
 *
 * Note that the ownership of the @p system pointer is <b>not</b> transferred
 * to this class. If you want to replace a system you have set before use
 * the following code:
 * <code>
 * OptimsocSystem* newSys = ...; // initialize the new system as usual
 *
 * OptimsocSystem* sys = OptimsocSystemFactory::currentSystem();
 * delete sys;
 * OptimsocSystemFactory::setCurrentSystem(newSys);
 * </code>
 *
 * If the current system changes the signal currentSystemChanged() is emitted.
 * To connecto to this class you need to get the singleton instance of this
 * class.
 * <code>
 * connect(OptimsocSystemFactory::instance(), SIGNAL(currentSystemChanged(OptimsocSystem*, OptimsocSystem*)),
 *         this, SLOT(someSlot()));
 * </code>
 *
 * @see instance()
 *
 * @param system The new default system.
 */
void OptimsocSystemFactory::setCurrentSystem(OptimsocSystem *system)
{
    if (s_currentSystem == system) {
        return;
    }

    OptimsocSystem* oldSystem = s_currentSystem;
    s_currentSystem = system;
    emit instance()->currentSystemChanged(oldSystem, s_currentSystem);
}

/**
 * Get the current default system
 *
 * @return
 */
OptimsocSystem* OptimsocSystemFactory::currentSystem()
{
    return s_currentSystem;
}

/**
 * Get the directory containing the OpTiMSoC System Description XML files
 *
 * We search the following directories, specified by environment variables:
 *
 * 1) $OPTIMSOC_SYSTEM_DESCRIPTIONS
 * 2) $OPTIMSOC/src/sw/host/system-descriptions
 *
 * @return the system description directory. The string might be null if no
 *         valid directory was found. Check with QString::isNull() before before
 *         using the value!
 */
QString OptimsocSystemFactory::getSysdescDir()
{
    QProcessEnvironment sysenv = QProcessEnvironment::systemEnvironment();
    if (sysenv.contains("OPTIMSOC_SYSTEM_DESCRIPTIONS")) {
        return sysenv.value("OPTIMSOC_SYSTEM_DESCRIPTIONS");
    }

    if (sysenv.contains("OPTIMSOC")) {
        return QString("%1/src/sw/host/system-descriptions")
                       .arg(sysenv.value("OPTIMSOC"));
    }

    qWarning("No system description path is set. Set the environment "
             "variable OPTIMSOC_SYSTEM_DESCRIPTIONS to a directory with "
             "the system description XML files.");
    return QString();
}
