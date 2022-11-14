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

string fn_mqo_support_header_text(const type_mqo_header* pheader) {
    stringstream ss;

    ss << DEFINE_MQO_TAG_HEADER_DOCUMENT << endl;
    ss << DEFINE_MQO_TAG_HEADER_FORMAT << " " << (*pheader).format << " ";
    ss << DEFINE_MQO_TAG_HEADER_VERSION << " " << (*pheader).version << endl;
    ss << DEFINE_MQO_TAG_HEADER_CODEPAGE << " " << (*pheader).codepage << endl;
    ss << endl;

    return ss.str();
}

string fn_mqo_support_scene_text(const type_mqo_scene* pscene) {
    stringstream ss;

    ss << "Scene {" << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_POS << " " << (*pscene).pos[0] << " " << (*pscene).pos[1] << " " << (*pscene).pos[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_LOOKAT << " " << (*pscene).lookat[0] << " " << (*pscene).lookat[1] << " " << (*pscene).lookat[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_HEAD << " " << (*pscene).head << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_PICH << " " << (*pscene).pich << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_BANK << " " << (*pscene).bank << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_ORTHO << " " << (*pscene).ortho << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_ZOOM2 << " " << (*pscene).zoom2 << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_AMB << " " << (*pscene).amb[0] << " " << (*pscene).amb[1] << " " << (*pscene).amb[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_FRONTCLIP << " " << (*pscene).frontclip << endl;
    ss << "\t" << DEFINE_MQO_TAG_SCENE_BACKCLIP << " " << (*pscene).backclip << endl;

    size_t n = (*pscene).dirlights.size();
    if (n > 0) {
        ss << "\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS << " " << (*pscene).dirlights.size() << " {" << endl;
        for (int32 i = 0; i < static_cast<int32>(n); i++) {
            ss << "\t\t" << (*pscene).dirlights[i].name << " {" << endl;
            ss << "\t\t\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS_DIR << " ";
            ss << (*pscene).dirlights[i].direction.x << " " << (*pscene).dirlights[i].direction.y << " " << (*pscene).dirlights[i].direction.z << endl;
            ss << "\t\t\t" << DEFINE_MQO_TAG_SCENE_DIRLIGHTS_COLOR << " ";
            ss << (*pscene).dirlights[i].color.r << " " << (*pscene).dirlights[i].color.g << " " << (*pscene).dirlights[i].color.b << endl;
            ss << "\t\t}" << endl;
        }
        ss << "\t}" << endl;
    }

    ss << "}" << endl;
    ss << endl;

    return ss.str();
}

string fn_mqo_support_object_text(const type_mqo_object* pobject) {
    stringstream ss;

    ss << "Object \"" << (*pobject).name << "\" {" << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_DEPTH << " " << (*pobject).depth << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_FOLDING << " " << (*pobject).folding << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_SCALE << " " << (*pobject).scale[0] << " " << (*pobject).scale[1] << " " << (*pobject).scale[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_ROTATION << " " << (*pobject).rotation[0] << " " << (*pobject).rotation[1] << " " << (*pobject).rotation[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_TRANSLATION << " " << (*pobject).translation[0] << " " << (*pobject).translation[1] << " " << (*pobject).translation[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_VISIBLE << " " << (*pobject).visible << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_LOCKING << " " << (*pobject).locking << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_SHADING << " " << (*pobject).shading << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_FACET << " " << (*pobject).facet << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_NORMALWEIGHT << " " << (*pobject).normal_weight << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_COLOR << " " << (*pobject).color << " " << (*pobject).color[1] << " " << (*pobject).color[2] << endl;
    ss << "\t" << DEFINE_MQO_TAG_OBJECT_COLORTYPE << " " << (*pobject).color_type << endl;

    auto size_max = static_cast<uint32>((*pobject).vertex.size());
    ss << "\tvertex " << size_max << " {" << endl;
    for (uint32 i = 0; i < size_max; i++) {
        double x = (*pobject).vertex[i].x;
        double y = (*pobject).vertex[i].y;
        double z = (*pobject).vertex[i].z;

        ss << "\t\t" << x << " " << y << " " << z << endl;
    }
    ss << "\t}" << endl;

    size_max = static_cast<uint32>((*pobject).face.size());
    ss << "\tface " << size_max << " {" << endl;
    for (uint32 i = 0; i < size_max; i++) {
        auto face = (*pobject).face[i];
        const auto n = static_cast<uint32>(face.vertex.size());
        ss << "\t\t" << n << " V( ";
        for (uint32 j = 0; j < face.vertex.size(); j++) {
            const auto index = face.vertex[j];
            ss << index << " ";
        }
        ss << ")";
        const auto m = face.material;
        if (m < INT16_MAX) {
            ss << " M(" << m << ")";
        }
        ss << endl;
    }
    ss << "\t}" << endl;

    ss << "}" << endl;
    ss << endl;

    return ss.str();
}

int32 fn_mqo_write(const type_mqo* pmqo, const string name) {
    // èëçû
    ofstream fout(name, ios::out);
    if (!fout) {
        return (-1);
    }

    fout << fn_mqo_support_header_text(&((*pmqo).header));
    fout << fn_mqo_support_scene_text(&((*pmqo).scene));
    fout << fn_mqo_support_object_text(&((*pmqo).object));

    fout.close();

    return (0);
}

int32 fn_stl_write(const type_mqo* pmqo, const string name) {
    auto vert = pmqo->object.vertex;
    auto faces = pmqo->object.face;

    // èëçû
    ofstream fout(name, ios::out);
    if (!fout) {
        return (-1);
    }

    fout << "solid notitle\n";
    for (uint32 i = 0; i < faces.size(); i++) {
        auto face = faces[i];
        auto ia = face.vertex[0];
        auto io = face.vertex[1];
        auto ib = face.vertex[2];
        auto va = vert[ia];
        auto vo = vert[io];
        auto vb = vert[ib];
        char str[128];

        fout << "facet normal 1 1 1\n";

        fout << "outer loop\n";
        sprintf_s(str, sizeof(str), "vertex %f %f %f\n", vb.x, vb.y, vb.z);
        fout << str;
        sprintf_s(str, sizeof(str), "vertex %f %f %f\n", vo.x, vo.y, vo.z);
        fout << str;
        sprintf_s(str, sizeof(str), "vertex %f %f %f\n", va.x, va.y, va.z);
        fout << str;
        fout << "endloop\n";

        fout << "endfacet\n";
    }
    
    fout << "endsolid notitle\n";
    fout.close();
    return (0);
}

type_mqo fn_mqo_create_default_parameter() {
    type_mqo mqo;

    type_mqo_direction d = {
        DEFINE_MQO_DEF_SCENE_LIGHT_DIR_X,
        DEFINE_MQO_DEF_SCENE_LIGHT_DIR_Y,
        DEFINE_MQO_DEF_SCENE_LIGHT_DIR_Z
    };
    type_mqo_color c = {
        DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_R,
        DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_G,
        DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_B
    };

    type_mqo_light light = { "light", d, c };

    mqo.scene.dirlights.push_back(light);

    return mqo;
}
