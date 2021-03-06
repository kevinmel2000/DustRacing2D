// This file belongs to the "MiniCore" game engine.
// Copyright (C) 2013 Jussi Lind <jussi.lind@iki.fi>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//

#ifndef MCMESHCONFIGLOADER_HH
#define MCMESHCONFIGLOADER_HH

#include "mcmeshmetadata.hh"

#include <vector>
#include <memory>

class QDomElement;
class QDomNode;

//! Loads the mesh config.
class MCMeshConfigLoader
{
public:

    //! Load all meshes found in filePath.
    //! \return true if succeeded.
    bool load(const std::string & filePath);

    //! Get mesh count.
    unsigned int meshCount() const;

    //! Get mesh data of given index.
    const MCMeshMetaData & mesh(unsigned int index) const;

private:

    typedef std::shared_ptr<MCMeshMetaData> MeshDataPtr;

    void parseAttributes(const QDomElement & element, MeshDataPtr newData, const std::string & baseModelPath);

    void parseChildNodes(const QDomNode & node, MeshDataPtr newData);

    std::vector<MeshDataPtr> m_meshes;
};

#endif // MCMESHCONFIGLOADER_HH
