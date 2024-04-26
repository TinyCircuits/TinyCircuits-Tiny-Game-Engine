import os
import shutil
import glob
import codecs


header_marker = "--- doc ---"
name_marker = "NAME: "
id_marker = "ID: "
desc_marker = "DESC: "
return_marker = "RETURN: "
param_marker = "PARAM: "
attr_marker = "ATTR: "
ovrr_marker = "OVRR: "


def find_nearest_newline(str, start_index):
    index_lf = str.find('\n', start_index)
    index_br = str.find('<br>', start_index)
    return max(index_lf, index_br)


def get_docstring_properties(docstring, marker, required=True):
    prop_start_index = 0
    prop_start_index = docstring.find(marker)
    properties = []

    while prop_start_index != -1:
        prop_start_index += len(marker)
        prop_end_index = find_nearest_newline(docstring, prop_start_index)
        prop = docstring[prop_start_index:prop_end_index]
        prop = prop.replace("\r", "")
        properties.append(prop)

        prop_start_index = docstring.find(marker, prop_start_index)
    
    if required and len(properties) == 0:
        return -1
    
    if len(properties) == 0:
        return None
    else:
        return properties


def get_attr_or_param_name_type_value(str):
    name_marker = "[name="
    value_marker = "[value="
    type_marker = "[type="

    name_start_index = str.find(name_marker)
    value_start_index = str.find(value_marker)
    type_start_index = str.find(type_marker)

    if name_start_index == -1:
        print("ERROR: attr or param does not have a name")
        return -1,0,0
    
    if value_start_index == -1:
        print("ERROR: attr or param does not have a value")
        return -1,0,0
    
    if type_start_index == -1:
        print("ERROR: attr or param does not have a type")
        return -1,0,0

    name_start_index += len(name_marker)
    value_start_index += len(value_marker)
    type_start_index += len(type_marker)
    
    name = str[name_start_index:str.find("]", name_start_index)]
    value = str[value_start_index:str.find("]", value_start_index)]
    type = str[type_start_index:str.find("]", type_start_index)]

    return name,value,type