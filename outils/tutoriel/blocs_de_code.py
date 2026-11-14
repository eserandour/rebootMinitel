# Ajoute de l'espace avant la première et après la dernière ligne de chaque bloc de code
import zipfile, sys, shutil
from lxml import etree
src, dst = sys.argv[1], sys.argv[2]
z = zipfile.ZipFile(src)
content = etree.fromstring(z.read('content.xml'))
NS = {'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0','text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
      'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0','fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0'}
S = lambda a: '{%s}%s' % (NS[a.split(':')[0]], a.split(':')[1])
auto = content.find('office:automatic-styles', NS)
code = {st.get(S('style:name')) for st in auto.findall('style:style', NS) if st.get(S('style:parent-style-name')) == 'Preformatted_20_Text'}
def mk(name, top, bottom):
    st = etree.SubElement(auto, S('style:style')); st.set(S('style:name'), name); st.set(S('style:family'), 'paragraph')
    st.set(S('style:parent-style-name'), 'Preformatted_20_Text')
    pp = etree.SubElement(st, S('style:paragraph-properties'))
    pp.set(S('fo:margin-top'), top); pp.set(S('fo:margin-bottom'), bottom)
    pp.set(S('fo:padding-top'), '0.12cm' if top != '0cm' else '0.04cm')
    pp.set(S('fo:padding-bottom'), '0.12cm' if bottom != '0cm' else '0.04cm')
    if bottom != '0cm': pp.set(S('fo:keep-with-next'), 'auto')
mk('CodeFirst', '0.2cm', '0cm'); mk('CodeLast', '0cm', '0.25cm'); mk('CodeOnly', '0.2cm', '0.25cm'); mk('CodeMid', '0cm', '0cm')
mk('CodeLastK', '0cm', '0.15cm'); mk('CodeOnlyK', '0.2cm', '0.15cm')
for nm in ('CodeLastK','CodeOnlyK'):
    st = auto.find("style:style[@style:name='%s']" % nm, NS)
    st.find('style:paragraph-properties', NS).set(S('fo:keep-with-next'), 'always')
capt = {st.get(S('style:name')) for st in auto.findall('style:style', NS) if st.get(S('style:parent-style-name')) == 'Capture'} | {'Capture'}
body = content.find('office:body/office:text', NS)
kids = list(body)
isc = [k.tag == S('text:p') and k.get(S('text:style-name')) in code for k in kids]
for i, k in enumerate(kids):
    if not isc[i]: continue
    first = i == 0 or not isc[i-1]; last = i == len(kids)-1 or not isc[i+1]
    nxt = kids[i+1] if i+1 < len(kids) else None
    k.set(S('text:style-name'), 'CodeOnly' if first and last else 'CodeFirst' if first else 'CodeLast' if last else 'CodeMid')
    # un bloc suivi d'une capture d'écran reste attaché à elle
    if last and nxt is not None and nxt.get(S('text:style-name')) in capt:
        k.set(S('text:style-name'), 'CodeOnlyK' if first else 'CodeLastK')
zout = zipfile.ZipFile(dst, 'w')
for item in z.infolist():
    data = z.read(item.filename)
    if item.filename == 'content.xml': data = etree.tostring(content, xml_declaration=True, encoding='utf-8')
    zout.writestr(item, data, compress_type=zipfile.ZIP_STORED if item.filename == 'mimetype' else zipfile.ZIP_DEFLATED)
zout.close()
print('blocs de code :', sum(1 for i in range(len(kids)) if isc[i] and (i == 0 or not isc[i-1])))
