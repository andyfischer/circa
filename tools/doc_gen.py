
DocsDump = 'docs.json'
DestinationDocs = 'gh-pages/docs'
DestinationIncludes = 'gh-pages/_includes/docs'

import json,math

# Load dumped data
docsDumpString = open(DocsDump).read()
everyEntry = json.loads(docsDumpString)

# Correlate data
termToEntry = {}
nameToEntry = {}

circa_allTypes = {'title':'All types', 'name':'all_types', 'items':[]}
circa_allFunctions = {'title':'All functions', 'name':'all_functions', 'items':[]}
improv_allTypes = {'title':'All types','name':'improv_types', 'items':[]}
improv_allFunctions = {'title':'All functions','name':'improv_functions','items':[]}

circaCategory = {'title':'Core','subcategories': [circa_allTypes, circa_allFunctions]}
improvCategory = {'title':'Improv','subcategories': [improv_allTypes, improv_allFunctions]}
majorCategories = [circaCategory, improvCategory]

for title in [ #['Color', 'color_tag'],
        'Comparison',
        'Containers',
        'Entropy','Internal', 'Math', 'Reflection', 'Stateful', 'Uncategorized']:

    if isinstance(title, list):
        name = title[1]
        title = title[0]
    else:
        name = title

    category = {'title':title, 'name':name, 'items':[]}
    circaCategory['subcategories'].append(category)

    nameToEntry[category['name']] = category

def everySubCategory():
    for cat in majorCategories:
        for subcat in cat['subcategories']:
            yield subcat

for entry in everyEntry:
    if entry['type'] == 'Function':
        circa_allFunctions['items'].append(entry)
    elif entry['type'] == 'Type':
        circa_allTypes['items'].append(entry)

    # Assign derived data

    # Fix names that will collide when they are case-insensitive
    if entry['name'] == 'list':
        entry['name'] = 'list_func'
    elif entry['name'] == 'set':
        entry['name'] = 'set_func'

    entry['title'] = entry['name']
    entry['filename'] = DestinationDocs + '/' + entry['name'] + '.md'
    entry['url'] = '/docs/' + entry['name'] + '.html'
    entry['linkHtml'] = '<a href="' + entry['url'] + '">' + entry['title'] + '</a>'
    entry['belongsToCategories'] = []
    entry['overloadEntries'] = []
    entry['typeMethods'] = []

    if 'tags' not in entry:
        entry['tags'] = []

    if 'term' in entry:
        if entry['term'] == "Term#null":
            continue
        termToEntry[entry['term']] = entry

    if 'name' in entry:
        nameToEntry[entry['name']] = entry

# Correlate overloaded functions with their contents
for entry in everyEntry:
    if 'isOverloaded' in entry and entry['isOverloaded']:
        for overload in entry['overloads']:
            term = overload
            if term not in termToEntry:
                print "couldn't find ref", term, "for overloaded func", entry['name']
                continue

            overloadEntry = termToEntry[term]
            entry['overloadEntries'].append(overloadEntry)

# Correlate methods with their owning types
for entry in everyEntry:
    if 'isMethod' in entry and entry['isMethod']:
        owningType = entry['inputTypes'][0]
        typeEntry = termToEntry[owningType]
        entry['methodOfType'] = typeEntry
        typeEntry['typeMethods'].append(entry)


# Derived data on categories
for cat in everySubCategory():
    cat['filename'] = DestinationDocs + '/' + cat['name'] + '.md'
    cat['url'] = '/docs/' + cat['name'] + '.html'

# Manually put functions into categories.
def addToCategory(entry, category):
    category['items'].append(entry)
    entry['belongsToCategories'].append(category)

def addNamesToCategory(nameList, categoryName):
    category = nameToEntry[categoryName]
    for name in nameList:
        if name not in nameToEntry:
            print("couldn't find name:", name)
            continue

        entry = nameToEntry[name]
        addToCategory(entry, category)

#addNamesToCategory(['Color','blend_color','random_color','hsv_to_rgb'], 'color_tag')
#addNamesToCategory(['darken','lighten'], 'color_tag')


addNamesToCategory(['rand','rand_i','rand_range'], 'Entropy')
addNamesToCategory(['seed','random_color','random_element','random_norm_vector'], 'Entropy')

addNamesToCategory(['abs','add','div','sub','ceil'], 'Math')
addNamesToCategory(['arcsin', 'arccos', 'arctan', 'average', 'bezier3', 'bezier4'], 'Math')
addNamesToCategory(['floor', 'log', 'magnitude', 'norm'], 'Math')
addNamesToCategory(['max','min','mod'], 'Math')
addNamesToCategory(['mult', 'neg','pow','rand','seed'], 'Math')
addNamesToCategory(['sin', 'sqr', 'sqrt', 'cos','tan','rotate_point','round'], 'Math')
addNamesToCategory(['rect_intersects_rect'], 'Math')
addNamesToCategory(['remainder','clamp','polar', 'point_distance'], 'Math')
addNamesToCategory(['smoothstep','smootherstep'], 'Math')
addNamesToCategory(['div_f','div_i','div_s'], 'Math')
addNamesToCategory(['Point','Point_i','Rect','Rect_i'], 'Math')
addNamesToCategory(['smooth_in_out','cube','perpendicular'], 'Math')
addNamesToCategory(['rand_i','rand_range','random_norm_vector'], 'Math')

addNamesToCategory(['equals','not_equals','greater_than', 'greater_than_eq'], 'Comparison')
addNamesToCategory(['less_than','less_than_eq'], 'Comparison')

addNamesToCategory(['Dict','Set','List','list','set'], 'Containers')

addNamesToCategory(['approach','approach_rect','delta','seed','toggle'], 'Stateful')
addNamesToCategory(['cycle','cycle_elements','once','changed'], 'Stateful')

addNamesToCategory(['extra_output','unknown_function','unknown_identifier'], 'Internal')
addNamesToCategory(['pack_state','unpack_state','pack_state_list_n','unpack_state_list_n'], 'Internal')
addNamesToCategory(['unpack_state_from_list','pack_state_to_list'], 'Internal')
addNamesToCategory(['unbounded_loop_finish','for'], 'Internal')
addNamesToCategory(['case','default_case','comment'], 'Internal')
addNamesToCategory(['input_placeholder','output_placeholder'], 'Internal')
addNamesToCategory(['test_oracle','test_spy'], 'Internal')
addNamesToCategory(['branch'], 'Internal')
addNamesToCategory(['static_error'], 'Internal')
addNamesToCategory(['switch','namespace','if_block'], 'Internal')
addNamesToCategory(['continue','break','discard','return'], 'Internal')
addNamesToCategory(['loop','loop_output','loop_iterator','loop_index'], 'Internal')
addNamesToCategory(['unrecognized_expr', 'overload_error_no_match'], 'Internal')
addNamesToCategory(['inputs_fit_function'], 'Internal')

addNamesToCategory(['Branch','Term','Interpreter','Frame'], 'Reflection')
addNamesToCategory(['make_interpreter','overload:get_contents'], 'Reflection')
addNamesToCategory(['lookup_branch_ref'], 'Reflection')
addNamesToCategory(['is_overloaded_func'], 'Reflection')

# For any overloaded function in a category, add its overloads to the same category
for entry in everyEntry:
    for overload in entry['overloadEntries']:
        for category in entry['belongsToCategories']:
            addToCategory(overload, category)

# For any type in a category, add its methods to the same category
for entry in everyEntry:
    for method in entry['typeMethods']:
        for category in entry['belongsToCategories']:
            addToCategory(method, category)

# Anything uncategorized goes in Uncategorized
uncategorizedCategory = nameToEntry['Uncategorized']
for entry in everyEntry:
    if not entry['belongsToCategories']:
        uncategorizedCategory['items'].append(entry)

# Check that no entries have duplicate filenames or urls
everyFilename = set()
everyUrl = set()

for entry in everyEntry:
    filename = entry['filename']
    url = entry['url']
    if url in everyUrl:
        print "duplicate url:", url
    if filename in everyFilename:
        print "duplicate filename:", filename

    everyFilename.add(filename)
    everyUrl.add(url)

def writeEntryPage(entry):
    out = []

    title = entry['title']

    out.append("---")
    out.append("title: "+ title)
    out.append("layout: docs")
    out.append("---")
    out.append("")

    if entry['type'] == 'Function':
        out.append("<h3><i>function</i> " + title + "</h3>")
    else:
        out.append("<h3><i>type</i> " + title + "</h3>")
    out.append("")

    if 'heading' in entry:
        heading = ""
        for element in entry['heading']:
            term = element[1]
            enableLink = False

            if term != entry['term'] and term in termToEntry:
                linkedEntry = termToEntry[term]
                enableLink = True
                heading += '<a href="' + linkedEntry['url'] + '">'

            heading += element[0]

            if enableLink:
                heading += '</a>'

        out.append(heading)
        out.append("")

    # Doc comments
    if 'topComments' in entry:
        for comment in entry['topComments']:
            if comment.startswith('--'):
                comment = comment[2:]
            comment = comment.strip()
            out.append("<p>" + comment + "</p>")
        out.append("")

    if entry['overloadEntries']:
        out.append("<h5>Overloaded function, includes these specific functions:</h5>")

        for entry in entry['overloadEntries']:
            out.append('<p>' + entry['linkHtml'] + '</p>')
        out.append("")

    if 'tags' in entry and entry['tags']:
        out.append("Tags:")
        out.append("")
    out.append("<p>" + entry['term'] + "</p>")

    return out

def writeLeftBar():
    out = []
    out.append('<div class="left_bar">')

    for majorCategory in majorCategories:
        out.append('<p><h1>' + majorCategory['title'] + '</h1></p>')

        for subcategory in majorCategory['subcategories']:
            out.append('  <p><h3><a href="' + subcategory['url'] + '">'
                + subcategory['title']
                + '</a></h3></p>')

    out.append('</div>')

    return out

def writeCategoryPage(category):
    title = category['title']

    out = []

    out.append("---")
    out.append("title: "+ title)
    out.append("layout: docs")
    out.append("---")
    out.append("")
    out.append("<h3>" + title + "</h3>")
    out.append("")

    # Divvy up functions into columns

    # Enforce a maximum of 4 columns
    MaxColumns = 4

    PreferredLimitPerColumn = 15

    items = category['items']
    items.sort(key = lambda i: i['title'].lower())

    columns = []

    maxItemsPerColumn = 0

    # If we can limit each column to 15 items or less, then minimize the column count
    if len(items) <= MaxColumns * PreferredLimitPerColumn:
        maxItemsPerColumn = PreferredLimitPerColumn

    else:
        # Otherwise, evenly balance the items per column
        maxItemsPerColumn = int(math.ceil(1.0 * len(items) / MaxColumns))

    for index in range(len(items)):
        columnIndex = index / maxItemsPerColumn
        if columnIndex >= len(columns):
            columns.append([])
        columns[columnIndex].append(items[index])

    out.append('')
    out.append('<table>')

    for index in range(maxItemsPerColumn):
        out.append('  <tr>')
        for column in columns:
            if index >= len(column):
                continue

            item = column[index]

            if 'linkHtml' not in item:
                print 'no linkHtml in', item['name']
                continue

            out.append('    <td>'+ item['linkHtml'] + '</td>')
        out.append('  </tr>')
    out.append('</table>')
    out.append('')

    return out

def writeFile(lines, file):
    f = open(file, 'w')
    for line in lines:
        f.write(line)
        f.write('\n')
    f.close()

# Left bar
writeFile(writeLeftBar(), DestinationIncludes + '/left_bar.html')

# Category pages
for cat in everySubCategory():
    lines = writeCategoryPage(cat)
    writeFile(lines, cat['filename'])

# Function-specific pages
for entry in everyEntry:
    lines = writeEntryPage(entry)
    writeFile(lines, entry['filename'])
