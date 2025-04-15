#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "mqo.h"

#define DEFINE_MQO_TAG_HEADER_DOCUMENT     ("Metasequoia Document")
#define DEFINE_MQO_TAG_HEADER_FORMAT       ("Format")
#define DEFINE_MQO_TAG_HEADER_VERSION      ("Ver")
#define DEFINE_MQO_TAG_HEADER_CODEPAGE     ("CodePage")

#define DEFINE_MQO_TAG_SCENE_POS             ("pos")
#define DEFINE_MQO_TAG_SCENE_LOOKAT          ("lookat")
#define DEFINE_MQO_TAG_SCENE_HEAD            ("head")
#define DEFINE_MQO_TAG_SCENE_PICH            ("pich")
#define DEFINE_MQO_TAG_SCENE_BANK            ("bank")
#define DEFINE_MQO_TAG_SCENE_ORTHO           ("ortho")
#define DEFINE_MQO_TAG_SCENE_ZOOM2           ("zoom2")
#define DEFINE_MQO_TAG_SCENE_AMB             ("amb")
#define DEFINE_MQO_TAG_SCENE_FRONTCLIP       ("frontclip")
#define DEFINE_MQO_TAG_SCENE_BACKCLIP        ("backclip")
#define DEFINE_MQO_TAG_SCENE_DIRLIGHTS       ("dirlights")
#define DEFINE_MQO_TAG_SCENE_DIRLIGHTS_DIR   ("dir")
#define DEFINE_MQO_TAG_SCENE_DIRLIGHTS_COLOR ("color")

#define DEFINE_MQO_TAG_OBJECT_DEPTH         ("depth")
#define DEFINE_MQO_TAG_OBJECT_FOLDING       ("folding")
#define DEFINE_MQO_TAG_OBJECT_SCALE         ("scale")
#define DEFINE_MQO_TAG_OBJECT_ROTATION      ("rotation")
#define DEFINE_MQO_TAG_OBJECT_TRANSLATION   ("translation")
#define DEFINE_MQO_TAG_OBJECT_VISIBLE       ("visible")
#define DEFINE_MQO_TAG_OBJECT_LOCKING       ("locking")
#define DEFINE_MQO_TAG_OBJECT_SHADING       ("shading")
#define DEFINE_MQO_TAG_OBJECT_FACET         ("facet")
#define DEFINE_MQO_TAG_OBJECT_NORMALWEIGHT  ("normal_weight")
#define DEFINE_MQO_TAG_OBJECT_COLOR         ("color")
#define DEFINE_MQO_TAG_OBJECT_COLORTYPE     ("color_type")

MQO::MQO() {
    type_direction d = {
        DEFINE_MQO_DEF_SCENE_LIGHT_DIR_X,
        DEFINE_MQO_DEF_SCENE_LIGHT_DIR_Y,
        DEFINE_MQO_DEF_SCENE_LIGHT_DIR_Z
    };
    type_color c = {
        DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_R,
        DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_G,
        DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_B
    };
    type_light light = { "light", d, c };
    m_scene.dirlights.push_back(light);
}

void
MQO::Write(const string file_path, const string marge_from) {
    ofstream fout;
    if (0 == marge_from.size()) {
        // èëçû
        fout = ofstream(file_path, ios::out);
        if (!fout) {
            return;
        }
        writeHeader(&fout);
        writeScene(&fout);
    } else {
        // í«ãL
        string marge_from_path;
        if (0 == marge_from.find_first_of('*')) {
            string marge_from_regix = marge_from.substr(1, marge_from.size());
            auto copy_len = file_path.size() - marge_from_regix.size();
            marge_from_path = file_path.substr(0, copy_len);
            marge_from_path.insert(copy_len, marge_from.substr(1, marge_from_regix.size()));
        } else {
            marge_from_path = marge_from;
        }
        fout = ofstream(marge_from_path, ios::app);
        if (!fout) {
            return;
        }
    }
    writeObject(&fout);
    writeLines(&fout);
    fout.close();
}

void
MQO::WriteStl(const string file_path, const string marge_from) {
    auto vert = m_object.vertex;
    auto faces = m_object.face;
    ofstream fout;
    if (0 == marge_from.size()) {
        // èëçû
        fout = ofstream(file_path, ios::out);
        if (!fout) {
            return;
        }
    } else {
        // í«ãL
        string marge_from_path;
        if (0 == marge_from.find_first_of('*')) {
            string marge_from_regix = marge_from.substr(1, marge_from.size());
            auto copy_len = file_path.size() - marge_from_regix.size();
            marge_from_path = file_path.substr(0, copy_len);
            marge_from_path.insert(copy_len, marge_from.substr(1, marge_from_regix.size()));
        } else {
            marge_from_path = marge_from;
        }
        fout = ofstream(marge_from_path, ios::app);
        if (!fout) {
            return;
        }
    }

    fout << "solid " << m_object.name << "\n";
    for (int32_t i = 0; i < faces.size(); i++) {
        auto face = faces[i];
        auto ia = face.vertex[0];
        auto io = face.vertex[1];
        auto ib = face.vertex[2];
        auto va = vert[ia];
        auto vo = vert[io];
        auto vb = vert[ib];
        char str[128];

        fout << "\tfacet\n";
        fout << "\touter loop\n";
        sprintf_s(str, sizeof(str), "\t\tvertex %f %f %f\n", vb.x, vb.z, vb.y);
        fout << str;
        sprintf_s(str, sizeof(str), "\t\tvertex %f %f %f\n", vo.x, vo.z, vo.y);
        fout << str;
        sprintf_s(str, sizeof(str), "\t\tvertex %f %f %f\n", va.x, va.z, va.y);
        fout << str;
        fout << "\tendloop\n";
        fout << "\tendfacet\n";
    }
    fout << "endsolid " << m_object.name << "\n";
    fout.close();
}

void
MQO::writeHeader(ofstream* p_fout) {
    *p_fout << DEFINE_MQO_TAG_HEADER_DOCUMENT << endl;
    *p_fout << DEFINE_MQO_TAG_HEADER_FORMAT << " " << m_header.format << " ";
    *p_fout << DEFINE_MQO_TAG_HEADER_VERSION << " " << m_header.version << endl;
    *p_fout << DEFINE_MQO_TAG_HEADER_CODEPAGE << " " << m_header.codepage << endl;
    *p_fout << endl;
}

void
MQO::writeScene(ofstream* p_fout) {
    *p_fout << "Scene {" << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_POS << " " << m_scene.pos[0] << " " << m_scene.pos[1] << " " << m_scene.pos[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_LOOKAT << " " << m_scene.lookat[0] << " " << m_scene.lookat[1] << " " << m_scene.lookat[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_HEAD << " " << m_scene.head << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_PICH << " " << m_scene.pich << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_BANK << " " << m_scene.bank << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_ORTHO << " " << m_scene.ortho << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_ZOOM2 << " " << m_scene.zoom2 << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_AMB << " " << m_scene.amb[0] << " " << m_scene.amb[1] << " " << m_scene.amb[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_FRONTCLIP << " " << m_scene.frontclip << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_BACKCLIP << " " << m_scene.backclip << endl;

    size_t n = m_scene.dirlights.size();
    if (n > 0) {
        *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS << " " << m_scene.dirlights.size() << " {" << endl;
        for (int32_t i = 0; i < static_cast<int32_t>(n); i++) {
            *p_fout << "\t\t" << m_scene.dirlights[i].name << " {" << endl;
            *p_fout << "\t\t\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS_DIR << " ";
            *p_fout << m_scene.dirlights[i].direction.x << " " << m_scene.dirlights[i].direction.y << " " << m_scene.dirlights[i].direction.z << endl;
            *p_fout << "\t\t\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS_COLOR << " ";
            *p_fout << m_scene.dirlights[i].color.r << " " << m_scene.dirlights[i].color.g << " " << m_scene.dirlights[i].color.b << endl;
            *p_fout << "\t\t}" << endl;
        }
        *p_fout << "\t}" << endl;
    }

    *p_fout << "}" << endl;
    *p_fout << endl;
}

void
MQO::writeObject(ofstream* p_fout) {
    *p_fout << "Object \"" << m_object.name << "\" {" << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_DEPTH << " " << m_object.depth << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_FOLDING << " " << m_object.folding << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_SCALE << " " << m_object.scale[0] << " " << m_object.scale[1] << " " << m_object.scale[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_ROTATION << " " << m_object.rotation[0] << " " << m_object.rotation[1] << " " << m_object.rotation[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_TRANSLATION << " " << m_object.translation[0] << " " << m_object.translation[1] << " " << m_object.translation[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_VISIBLE << " " << m_object.visible << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_LOCKING << " " << m_object.locking << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_SHADING << " " << m_object.shading << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_FACET << " " << m_object.facet << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_NORMALWEIGHT << " " << m_object.normal_weight << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_COLOR << " " << m_object.color << " " << m_object.color[1] << " " << m_object.color[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_COLORTYPE << " " << m_object.color_type << endl;

    auto size_max = static_cast<int32_t>(m_object.vertex.size());
    *p_fout << "\tvertex " << size_max << " {" << endl;
    for (int32_t i = 0; i < size_max; i++) {
        auto x = m_object.vertex[i].x;
        auto y = m_object.vertex[i].y;
        auto z = m_object.vertex[i].z;
        *p_fout << "\t\t" << x << " " << y << " " << z << endl;
    }
    *p_fout << "\t}" << endl;

    size_max = static_cast<uint32_t>(m_object.face.size());
    *p_fout << "\tface " << size_max << " {" << endl;
    for (int32_t i = 0; i < size_max; i++) {
        auto face = m_object.face[i];
        const auto n = static_cast<uint32_t>(face.vertex.size());
        *p_fout << "\t\t" << n << " V( ";
        for (int32_t j = 0; j < face.vertex.size(); j++) {
            const auto index = face.vertex[j];
            *p_fout << index << " ";
        }
        *p_fout << ")";
        const auto m = face.material;
        if (m < INT16_MAX) {
            *p_fout << " M(" << m << ")";
        }
        *p_fout << endl;
    }
    *p_fout << "\t}" << endl;

    *p_fout << "}" << endl;
    *p_fout << endl;
}

void
MQO::writeLines(ofstream* p_fout) {
    for (int32_t j = 0; j < m_object.lines.size(); j++) {
        auto line = m_object.lines[j];
        auto sz = line.size();
        auto va = m_object.vertex[line[0]];
        for (auto i = sz - 1; 1 <= i; i--) {
            auto vb = m_object.vertex[line[i]];
            *p_fout << "Object \"" << m_object.name << "_line" << j << "_" << (sz - i) << "\" {" << endl;
            *p_fout << "\tvertex " << 2 << " {" << endl;
            *p_fout << "\t\t" << va.x << " " << va.y << " " << va.z << endl;
            *p_fout << "\t\t" << vb.x << " " << vb.y << " " << vb.z << endl;
            *p_fout << "\t}" << endl;
            *p_fout << "\tface " << 2 << " {" << endl;
            *p_fout << "\t\t" << "2 V( 0 1 )" << endl;
            *p_fout << "\t}" << endl;
            *p_fout << "}" << endl;
            *p_fout << endl;
            va = vb;
        }
    }
}

