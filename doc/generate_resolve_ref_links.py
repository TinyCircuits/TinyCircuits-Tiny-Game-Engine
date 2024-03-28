import os
import shutil
import glob
import codecs
from generate_common import get_attr_or_param_name_type_value, get_docstring_properties, header_marker, name_marker, desc_marker, return_marker, attr_marker, param_marker, find_nearest_newline

# After all markdown fragments have been made, go through and replace all ref_link:X with links

ref_link = "{ref_link:"

# Get paths from this work directory to each file in 'build'
paths = glob.glob('build/**/*', recursive=True)

for path in paths:
    if os.path.isfile(path):
        # Open each file and get contents
        file = codecs.open(path, 'r', 'utf-8', errors='replace')
        file_contents = file.read()

        last_ref_link_index = 0
        last_ref_link_index = file_contents.find(ref_link)

        while last_ref_link_index != -1:
            link_start_index = last_ref_link_index + len(ref_link)
            link_end_index = file_contents.find("}", link_start_index)

            link_name = file_contents[link_start_index:link_end_index]
            link_dest = link_name
            link_after = ""

            # Need to check if the ref_link is a function, if
            # so, need return type and parameters with types too
            fragment_path = "build/" + link_name + ".html"
            if os.path.isfile(fragment_path):
                file_fragment = codecs.open(fragment_path, 'r', 'utf-8', errors='replace')
                file_fragments_content = file_fragment.read()
                file_fragment.close()

                # If there is a return value, then must be a function
                ret = get_docstring_properties(file_fragments_content, "<b>Return Value</b>: ", False)
                if ret != None:
                    link_name = file_fragments_content[0:find_nearest_newline(file_fragments_content, 0)]
                    link_name = link_name.replace("\r", "")
                    link_after = " [returns: " + ret[0] + "]"
            else:
                print("ERROR: Looking for fragment: `" + fragment_path + "` from `" + path + "`")
                raise "ERROR: Could not find fragment"

            # link_contents = "[" + link_name + "]" + "(" + link_dest  + ".html)" + link_after
            link_contents = "<a href=\"" + link_dest + ".html\">" + link_name + "</a>" + link_after
            file_contents = file_contents.replace(file_contents[last_ref_link_index:link_end_index+1], link_contents)

            last_ref_link_index += len(ref_link)
            last_ref_link_index = file_contents.find(ref_link, last_ref_link_index)

        file.close()

        file = codecs.open(path, 'w', 'utf-8', errors='replace')
        
        file.write(file_contents)
        file.write("<br><br><a href=\"../landing.html\">landing</a>")
        file.close()