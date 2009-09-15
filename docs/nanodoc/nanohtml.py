
"""

Json document is layed out like this:

headers
  title
packages
  name
  contents

"""

def to_ident(name):
    return name.strip(' ()').lower()

def packageListHTML(packages):
    html = "<div id=\"packages\">"
    html += "<div id=\"packagestitle\">Packages</div>"
    html += "<div id=\"nanobutton\">"
    html += "<ul>"
    
    # Leftmost column: one item for each package
    for package in packages:
        name = package['name']
        html += ("<li><a href=\"javascript:showPackageContents('%s_contents')\">%s</a></li>"
                % (to_ident(name), name))

    html += "</ul>"
    html += "</div>"
    html += "</div>"
    html += "\n"
    return html

def makeHTML(dom):

    def all_packages():
        for module in dom['packages']:
            yield module

    def all_module_contents():
        for module in dom['packages']:
            for item in module['contents']:
                yield item

    def all_functions():
        for item in all_module_contents():
            if 'function' in item:
                yield item

    html = ""
    
    html += '<html>\n'
    html += '\t<head>\n'
    html += '\t\t<title>nanodoc</title>\n'
    html += '\t\t<link rel="stylesheet" type="text/css" href="nanodoc.css" />\n'
    html += '\t\t<script type="text/javascript" src="nanodoc.js"></script>'
    html += "\t</head>\n"
    html += "\t<body>\n"
    html += '\t\t<div id="nanohead"><div id="nanotitle">' + dom['headers']['title'] + ' API documentation</div></div>\n'
    html += '\t\t<div id="nanobody">\n'

    html += packageListHTML(all_packages())

    # 2nd and 3rd columns: contents of packages
    for package in all_packages():
        package_name = package['name']

        html += ("<div style=\"display:none\" class=\"package_contents\" id=\"%s_contents\">"
                % to_ident(package_name))
        html += "<div id=\"modulestitle\">%s</div>" % package_name
        html += "<div id=\"nanobutton\">"

        html += '<div class="package_contents_bar" style="float:left">'

        html += "<ul>"

        for item in package['contents']:
            item_name = item['name']

            html += ("<li><a href=\"javascript:showSubContents('%s_sub')\">%s</a></li>"
                    % (to_ident(item_name), item_name))
        html += "</ul>"

        html += "</div>"

        html += '<div class="package_contents_bar" style="float:right">'

        html += "<ul>"

        for item in package['contents']:
            item_name = item['name']

            html += ("<li><a href=\"javascript:showSubContents('%s_sub')\">%s</a></li>"
                    % (to_ident(item_name), item_name))
        html += "</ul>"

        html += "</div>"

        html += "</div>"
        html += "</div>"

    html += "\n"

    # Rightmost item: actual documentation

    for func in all_functions():
        name = func['name']
        type_name = "Type"

        html += "<div style=\"display:none\" class=\"moduledetail\" id=\"%s_sub\">" % to_ident(name)
        html +=    "<div id=\"moduledetailtitle\">%s</div>" % name

        html += "<div id=\"detailcontent\"> <span class=\"hl_type\">%s</span> %s<span class=\"hl_func\">%s </span>" % (type_name, "", name)
        html += "("
        paramdescs = ""
        params = []
        description = ""
        returns = ""
        if len(params) > 0:
            params_html = []
            for subitem in item.getElementsByTagName('param'):
                if subitem.attributes["isPtr"].value == "True":
                    ptrFlag = "*"
                else:
                    ptrFlag = ""
                params_html.append("<span class=\"hl_type\">%s</span> %s<span class=\"hl_param\">%s</span>" % (type_name, "", name))
                paramdescs += "<div class=\"hl_type_b\">%s</div><div class=\"ptr\">%s</div><div class=\"hl_param_b\">%s</div><div class=\"hl_paramdesc\">%s</div>" % (type_name, "", name, "")
            html += ", ".join(params_html)
        html += ")</div>"
        
        if description:
            html +=    "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Description</div> <div class=\"detailbox\">%s.</div></div>" % description
    
        if returns:
            html +=    "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Return value</div> <div class=\"detailbox\">%s.</div></div>" % returns
    
        if len(paramdescs) > 0:
            html += "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Parameters</div><div class=\"detailbox\">"
            html += paramdescs
            html += "</div></div>"
        html += "</div>"

    html += "\n"

    return html

    for item in dom.documentElement.getElementsByTagName('struct'):
        html += "<div style=\"display:none\" class=\"moduledetail\" id=\"%s_sub\">" % (item.attributes["name"].value.strip(" ").lower())
        html +=    "<div id=\"moduledetailtitle\">%s</div>" % (item.attributes["name"].value)
        html += "<div id=\"detailcontent\"> <span class=\"hl_type\">typedef struct</span> {<br/>"
        paramdescs = ""
        if len(item.getElementsByTagName('param')) > 0:
            params = []
            for subitem in item.getElementsByTagName('param'):
                if subitem.attributes["isPtr"].value == "True":
                    ptrFlag = "*"
                else:
                    ptrFlag = ""
                params.append("<span class=\"hl_type\">%s</span> %s<span class=\"hl_param\">%s</span>" % (subitem.attributes["type"].value, ptrFlag, subitem.attributes["name"].value))
                paramdescs += "<div class=\"hl_type_b\">%s</div><div class=\"ptr\">%s</div><div class=\"hl_param_b\">%s</div><div class=\"hl_paramdesc\">%s</div>" % (subitem.attributes["type"].value, ptrFlag, subitem.attributes["name"].value, subitem.childNodes[0].data)
            html += ";<br/> ".join(params)
        html += ";<br/>} %s;</div>" % (item.attributes["name"].value)
        
        if len(item.getElementsByTagName('desc')) > 0:
            html +=    "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Description</div> <div class=\"detailbox\">%s.</div></div>" % (item.getElementsByTagName('desc')[0].childNodes[0].data)
        
        if len(paramdescs) > 0:
            html += "<div id=\"detailcontent\"><div class=\"detailboxdesc\"> Parameters</div><div class=\"detailbox\">"
            html += paramdescs
            html += "</div></div>"
        html += "</div>"


    html += "\t\t<div>\n"
    html += "\t</body>\n"
    html += "\t</html>\n"
    return html

from optparse import OptionParser
options = OptionParser()
options.add_option('--dom', dest = 'dom_filename')
options.add_option('--output', dest = 'output_filename')
(cl_options, cl_args) = options.parse_args()

def load_json_dom(filename):
    f = open(filename, 'r')
    file_contents = f.read()
    f.close()

    import json
    dom = json.loads(file_contents)
    return dom

def write_text_file(filename, contents):
    output_file = open(filename, 'w')
    output_file.write(contents)
    output_file.write("\n")

dom = load_json_dom(cl_options.dom_filename)
html = makeHTML(dom)
write_text_file(cl_options.output_filename, html)
