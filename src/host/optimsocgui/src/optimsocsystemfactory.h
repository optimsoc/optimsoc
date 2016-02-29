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

#ifndef OPTIMSOCSYSTEMFACTORY_H
#define OPTIMSOCSYSTEMFACTORY_H

#include <QObject>

#include "optimsocsystem.h"

class OptimsocSystemFactory : public QObject
{
Q_OBJECT
public:
    static OptimsocSystemFactory* instance();

    static OptimsocSystem* createSystemFromId(int systemId);
    static void setCurrentSystem(OptimsocSystem *system);
    static OptimsocSystem* currentSystem();
    static QString getSysdescDir();

signals:
    /**
     * The current system has changed
     *
     * @see currentSystem()
     * @see setCurrentSystem()
     *
     * @param oldSystem the old system which is being replaced
     * @param newSystem the new "default" (current) system
     */
    void currentSystemChanged(OptimsocSystem* oldSystem, OptimsocSystem *newSystem);

public slots:

private:
    OptimsocSystemFactory(QObject *parent = 0);
    static OptimsocSystem* s_currentSystem;
    static OptimsocSystemFactory* s_instance;

};

#endif // OPTIMSOCSYSTEMFACTORY_H
