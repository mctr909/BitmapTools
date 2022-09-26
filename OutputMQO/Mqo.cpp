#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#include "../CommonLib/Bitmap.h"
#include "mqo.h"
#include "ExtPolyline.h"

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
        const auto n = static_cast<uint32>((*pobject).face[i].vertex.size());
        const auto s = (*pobject).face[i].vertex[0];
        const auto e = (*pobject).face[i].vertex[1];
        const auto           m = (*pobject).face[i].material;
        ss << "\t\t" << n << " V(" << s << " " << e << ")";
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
