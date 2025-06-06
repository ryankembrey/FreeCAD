/***************************************************************************
 *   Copyright (c) 2023 David Carter <dcarter@david.carter.ca>             *
 *                                                                         *
 *   This file is part of FreeCAD.                                         *
 *                                                                         *
 *   FreeCAD is free software: you can redistribute it and/or modify it    *
 *   under the terms of the GNU Lesser General Public License as           *
 *   published by the Free Software Foundation, either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   FreeCAD is distributed in the hope that it will be useful, but        *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU      *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with FreeCAD. If not, see                               *
 *   <https://www.gnu.org/licenses/>.                                      *
 *                                                                         *
 **************************************************************************/

#include "PreCompiled.h"

#include "Model.h"
#include "ModelLibrary.h"
#include "ModelManager.h"
#include "ModelManagerPy.h"
#include "ModelPy.h"

#include "ModelManagerPy.cpp"

using namespace Materials;

// returns a string which represents the object e.g. when printed in python
std::string ModelManagerPy::representation() const
{
    std::stringstream str;
    str << "<ModelManager object at " << getModelManagerPtr() << ">";

    return str.str();
}

PyObject* ModelManagerPy::PyMake(struct _typeobject*, PyObject*, PyObject*)  // Python wrapper
{
    // never create such objects with the constructor
    return new ModelManagerPy(&(ModelManager::getManager()));
}

// constructor method
int ModelManagerPy::PyInit(PyObject* /*args*/, PyObject* /*kwd*/)
{
    return 0;
}

PyObject* ModelManagerPy::getModel(PyObject* args)
{
    char* uuid {};
    if (!PyArg_ParseTuple(args, "s", &uuid)) {
        return nullptr;
    }

    try {
        auto model = getModelManagerPtr()->getModel(QString::fromStdString(uuid));
        return new ModelPy(new Model(*model));
    }
    catch (ModelNotFound const&) {
        QString error = QStringLiteral("Model not found:\n");
        auto _modelMap = getModelManagerPtr()->getModels();
        error += QStringLiteral("ModelMap:\n");
        for (auto itp = _modelMap->begin(); itp != _modelMap->end(); itp++) {
            error += QStringLiteral("\t_modelMap[") + itp->first
                + QStringLiteral("] = '") + itp->second->getName()
                + QStringLiteral("'\n");
        }
        error += QStringLiteral("\tuuid = '") + QString::fromStdString(uuid)
            + QStringLiteral("'\n");
        PyErr_SetString(PyExc_LookupError, error.toStdString().c_str());
        return nullptr;
    }
    catch (Uninitialized const&) {
        PyErr_SetString(PyExc_LookupError, "Uninitialized model list");
        return nullptr;
    }
}

PyObject* ModelManagerPy::getModelByPath(PyObject* args)
{
    char* path {};
    const char* lib = "";
    if (!PyArg_ParseTuple(args, "s|s", &path, &lib)) {
        return nullptr;
    }

    std::string libPath(lib);
    if (libPath.length() > 0) {
        try {
            auto model = getModelManagerPtr()->getModelByPath(QString::fromStdString(path),
                                                              QString::fromStdString(libPath));
            return new ModelPy(new Model(*model));
        }
        catch (ModelNotFound const&) {
            PyErr_SetString(PyExc_LookupError, "Model not found");
            return nullptr;
        }
    }

    try {
        auto model = getModelManagerPtr()->getModelByPath(QString::fromStdString(path));
        return new ModelPy(new Model(*model));
    }
    catch (ModelNotFound const&) {
        PyErr_SetString(PyExc_LookupError, "Model not found");
        return nullptr;
    }
}

Py::List ModelManagerPy::getModelLibraries() const
{
    auto libraries = getModelManagerPtr()->getLibraries();
    Py::List list;

    for (auto it = libraries->begin(); it != libraries->end(); it++) {
        auto lib = *it;
        Py::Tuple libTuple(3);
        libTuple.setItem(0, Py::String(lib->getName().toStdString()));
        libTuple.setItem(1, Py::String(lib->getDirectoryPath().toStdString()));
        libTuple.setItem(2, Py::Bytes(lib->getIcon().data(), lib->getIcon().size()));
        libTuple.setItem(3, Py::Boolean(lib->isReadOnly()));

        list.append(libTuple);
    }

    return list;
}

Py::List ModelManagerPy::getLocalModelLibraries() const
{
    auto libraries = getModelManagerPtr()->getLocalLibraries();
    Py::List list;

    for (auto it = libraries->begin(); it != libraries->end(); it++) {
        auto lib = *it;
        Py::Tuple libTuple(3);
        libTuple.setItem(0, Py::String(lib->getName().toStdString()));
        libTuple.setItem(1, Py::String(lib->getDirectoryPath().toStdString()));
        libTuple.setItem(2, Py::Bytes(lib->getIcon().data(), lib->getIcon().size()));
        libTuple.setItem(3, Py::Boolean(lib->isReadOnly()));

        list.append(libTuple);
    }

    return list;
}

Py::Dict ModelManagerPy::getModels() const
{
    auto models = getModelManagerPtr()->getModels();
    Py::Dict dict;

    for (auto it = models->begin(); it != models->end(); it++) {
        QString key = it->first;
        auto model = it->second;

        PyObject* modelPy = new ModelPy(new Model(*model));
        dict.setItem(Py::String(key.toStdString()), Py::Object(modelPy, true));
    }

    return dict;
}

PyObject* ModelManagerPy::getCustomAttributes(const char* /*attr*/) const
{
    return nullptr;
}

int ModelManagerPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0;
}
