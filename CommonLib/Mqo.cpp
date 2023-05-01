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
    scene.dirlights.push_back(light);
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
    fout.close();
}

void
MQO::WriteStl(const string file_path, const string marge_from) {
    auto vert = object.vertex;
    auto faces = object.face;
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

    fout << "solid " << object.name << "\n";
    for (uint32 i = 0; i < faces.size(); i++) {
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
    fout << "endsolid " << object.name << "\n";
    fout.close();
}

void
MQO::writeHeader(ofstream* p_fout) {
    *p_fout << DEFINE_MQO_TAG_HEADER_DOCUMENT << endl;
    *p_fout << DEFINE_MQO_TAG_HEADER_FORMAT << " " << header.format << " ";
    *p_fout << DEFINE_MQO_TAG_HEADER_VERSION << " " << header.version << endl;
    *p_fout << DEFINE_MQO_TAG_HEADER_CODEPAGE << " " << header.codepage << endl;
    *p_fout << endl;
}

void
MQO::writeScene(ofstream* p_fout) {
    *p_fout << "Scene {" << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_POS << " " << scene.pos[0] << " " << scene.pos[1] << " " << scene.pos[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_LOOKAT << " " << scene.lookat[0] << " " << scene.lookat[1] << " " << scene.lookat[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_HEAD << " " << scene.head << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_PICH << " " << scene.pich << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_BANK << " " << scene.bank << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_ORTHO << " " << scene.ortho << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_ZOOM2 << " " << scene.zoom2 << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_AMB << " " << scene.amb[0] << " " << scene.amb[1] << " " << scene.amb[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_FRONTCLIP << " " << scene.frontclip << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_BACKCLIP << " " << scene.backclip << endl;

    size_t n = scene.dirlights.size();
    if (n > 0) {
        *p_fout << "\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS << " " << scene.dirlights.size() << " {" << endl;
        for (int32 i = 0; i < static_cast<int32>(n); i++) {
            *p_fout << "\t\t" << scene.dirlights[i].name << " {" << endl;
            *p_fout << "\t\t\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS_DIR << " ";
            *p_fout << scene.dirlights[i].direction.x << " " << scene.dirlights[i].direction.y << " " << scene.dirlights[i].direction.z << endl;
            *p_fout << "\t\t\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS_COLOR << " ";
            *p_fout << scene.dirlights[i].color.r << " " << scene.dirlights[i].color.g << " " << scene.dirlights[i].color.b << endl;
            *p_fout << "\t\t}" << endl;
        }
        *p_fout << "\t}" << endl;
    }

    *p_fout << "}" << endl;
    *p_fout << endl;
}

void
MQO::writeObject(ofstream* p_fout) {
    *p_fout << "Object \"" << object.name << "\" {" << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_DEPTH << " " << object.depth << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_FOLDING << " " << object.folding << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_SCALE << " " << object.scale[0] << " " << object.scale[1] << " " << object.scale[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_ROTATION << " " << object.rotation[0] << " " << object.rotation[1] << " " << object.rotation[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_TRANSLATION << " " << object.translation[0] << " " << object.translation[1] << " " << object.translation[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_VISIBLE << " " << object.visible << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_LOCKING << " " << object.locking << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_SHADING << " " << object.shading << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_FACET << " " << object.facet << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_NORMALWEIGHT << " " << object.normal_weight << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_COLOR << " " << object.color << " " << object.color[1] << " " << object.color[2] << endl;
    *p_fout << "\t" << DEFINE_MQO_TAG_OBJECT_COLORTYPE << " " << object.color_type << endl;

    auto size_max = static_cast<uint32>(object.vertex.size());
    *p_fout << "\tvertex " << size_max << " {" << endl;
    for (uint32 i = 0; i < size_max; i++) {
        double x = object.vertex[i].x;
        double y = object.vertex[i].y;
        double z = object.vertex[i].z;
        *p_fout << "\t\t" << x << " " << y << " " << z << endl;
    }
    *p_fout << "\t}" << endl;

    size_max = static_cast<uint32>(object.face.size());
    *p_fout << "\tface " << size_max << " {" << endl;
    for (uint32 i = 0; i < size_max; i++) {
        auto face = object.face[i];
        const auto n = static_cast<uint32>(face.vertex.size());
        *p_fout << "\t\t" << n << " V( ";
        for (uint32 j = 0; j < face.vertex.size(); j++) {
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
