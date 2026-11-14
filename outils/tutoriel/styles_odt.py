# Construit sortie/reference.odt : les styles du tutoriel (titres, blocs de code, encadrés, A4)
from lxml import etree
import shutil, zipfile, os
NS = {
 'style':'urn:oasis:names:tc:opendocument:xmlns:style:1.0',
 'fo':'urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0',
 'office':'urn:oasis:names:tc:opendocument:xmlns:office:1.0',
 'svg':'urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0',
 'text':'urn:oasis:names:tc:opendocument:xmlns:text:1.0',
}
DECL = ' '.join(f'xmlns:{k}="{v}"' for k,v in NS.items())
import subprocess, tempfile
ICI = os.path.dirname(os.path.abspath(__file__))
TRAVAIL = tempfile.mkdtemp()
subprocess.run(['pandoc', '-o', os.path.join(TRAVAIL, 'defaut.odt'), '--print-default-data-file', 'reference.odt'], check=True)
zipfile.ZipFile(os.path.join(TRAVAIL, 'defaut.odt')).extractall(os.path.join(TRAVAIL, 'ref'))
tree = etree.parse(os.path.join(TRAVAIL, 'ref', 'styles.xml')); root = tree.getroot()
styles = root.find('office:styles', NS)
FONT='Liberation Sans'; MONO='DejaVu Sans Mono'
def frag(x):
    return etree.fromstring(f'<root {DECL}>{x}</root>')[0]
def put(xml):
    el = frag(xml)
    name = el.get(f'{{{NS["style"]}}}name'); fam = el.get(f'{{{NS["style"]}}}family')
    for old in styles.findall('style:style', NS):
        if old.get(f'{{{NS["style"]}}}name') == name and old.get(f'{{{NS["style"]}}}family') == fam:
            styles.remove(old)
    styles.append(el)
# polices
ffd = root.find('office:font-face-decls', NS)
for nm, gen, pitch in [(FONT,'swiss','variable'),(MONO,'modern','fixed')]:
    ffd.append(frag(f'<style:font-face style:name="{nm}" svg:font-family="&apos;{nm}&apos;" style:font-family-generic="{gen}" style:font-pitch="{pitch}"/>'))
# style par défaut
for d in styles.findall('style:default-style', NS):
    if d.get(f'{{{NS["style"]}}}family') == 'paragraph':
        styles.remove(d)
styles.insert(0, frag(f'<style:default-style style:family="paragraph"><style:paragraph-properties style:tab-stop-distance="1.25cm" style:writing-mode="page"/><style:text-properties style:font-name="{FONT}" fo:font-size="10.5pt" fo:language="fr" fo:country="FR" style:font-name-asian="{FONT}" style:font-name-complex="{FONT}" fo:hyphenate="true" fo:hyphenation-remain-char-count="3" fo:hyphenation-push-char-count="3" style:letter-kerning="true"/></style:default-style>'))
put(f'<style:style style:name="Standard" style:family="paragraph" style:class="text"><style:text-properties style:font-name="{FONT}" fo:font-size="10.5pt" fo:color="#222222"/></style:style>')
put('<style:style style:name="Text_20_body" style:display-name="Text body" style:family="paragraph" style:parent-style-name="Standard" style:class="text"><style:paragraph-properties fo:margin-top="0cm" fo:margin-bottom="0.22cm" fo:line-height="128%" fo:text-align="justify" fo:orphans="2" fo:widows="2"/></style:style>')
put('<style:style style:name="First_20_paragraph" style:display-name="First paragraph" style:family="paragraph" style:parent-style-name="Text_20_body" style:next-style-name="Text_20_body" style:class="text"/>')
put(f'<style:style style:name="Heading" style:family="paragraph" style:parent-style-name="Standard" style:next-style-name="Text_20_body" style:class="text"><style:paragraph-properties fo:margin-top="0.5cm" fo:margin-bottom="0.2cm" fo:keep-with-next="always"/><style:text-properties style:font-name="{FONT}" fo:font-weight="bold" fo:color="#1F3A5F" fo:hyphenate="false"/></style:style>')
put('<style:style style:name="Heading_20_1" style:display-name="Heading 1" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="1" style:class="text"><style:paragraph-properties fo:break-before="page" fo:margin-top="0cm" fo:margin-bottom="0.5cm" fo:padding-bottom="0.15cm" fo:border-bottom="0.06cm solid #2E5C8A"/><style:text-properties fo:font-size="22pt"/></style:style>')
put('<style:style style:name="Heading_20_2" style:display-name="Heading 2" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="2" style:class="text"><style:paragraph-properties fo:margin-top="0.55cm" fo:margin-bottom="0.2cm"/><style:text-properties fo:font-size="14pt" fo:color="#2E5C8A"/></style:style>')
put('<style:style style:name="Heading_20_3" style:display-name="Heading 3" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Text_20_body" style:default-outline-level="3" style:class="text"><style:text-properties fo:font-size="12pt"/></style:style>')
put(f'<style:style style:name="Preformatted_20_Text" style:display-name="Preformatted Text" style:family="paragraph" style:parent-style-name="Standard" style:class="html"><style:paragraph-properties fo:margin-left="0.2cm" fo:margin-right="0.2cm" fo:margin-top="0cm" fo:margin-bottom="0cm" fo:background-color="#EDF1F6" fo:padding-left="0.3cm" fo:padding-right="0.2cm" fo:padding-top="0.05cm" fo:padding-bottom="0.05cm" fo:border-left="0.12cm solid #2E5C8A" fo:border-right="none" fo:border-top="none" fo:border-bottom="none" fo:keep-with-next="always" style:join-border="true"/><style:text-properties style:font-name="{MONO}" fo:font-size="9.5pt" fo:color="#10243A" fo:hyphenate="false"/></style:style>')
put(f'<style:style style:name="Source_Text" style:family="text"><style:text-properties style:font-name="{MONO}" fo:font-size="9.5pt" fo:color="#10243A" fo:background-color="#EDF1F6"/></style:style>')
put('<style:style style:name="Capture" style:family="paragraph" style:parent-style-name="Standard"><style:paragraph-properties fo:text-align="center" fo:margin-top="0.25cm" fo:margin-bottom="0.35cm"/></style:style>')
for nm, bg, bd in [('Astuce','#E6F3E8','#3A8F4A'),('Attention','#FCEEDC','#D9822B')]:
    put(f'<style:style style:name="{nm}" style:family="paragraph" style:parent-style-name="Text_20_body"><style:paragraph-properties fo:margin-left="0.2cm" fo:margin-right="0.2cm" fo:margin-top="0.25cm" fo:margin-bottom="0.35cm" fo:background-color="{bg}" fo:padding="0.25cm" fo:border-left="0.15cm solid {bd}" fo:border-right="none" fo:border-top="none" fo:border-bottom="none" fo:keep-together="always" fo:text-align="left"/></style:style>')
put('<style:style style:name="Title" style:family="paragraph" style:parent-style-name="Heading" style:next-style-name="Subtitle" style:class="chapter"><style:paragraph-properties fo:text-align="center" fo:margin-top="6cm" fo:margin-bottom="0.6cm"/><style:text-properties fo:font-size="34pt" fo:color="#1F3A5F"/></style:style>')
put('<style:style style:name="Subtitle" style:family="paragraph" style:parent-style-name="Heading" style:class="chapter"><style:paragraph-properties fo:text-align="center" fo:margin-bottom="3cm"/><style:text-properties fo:font-size="16pt" fo:font-weight="normal" fo:color="#2E5C8A"/></style:style>')
put('<style:style style:name="Date" style:family="paragraph" style:parent-style-name="Standard" style:class="html"><style:paragraph-properties fo:text-align="center" fo:margin-bottom="1cm"/><style:text-properties fo:font-size="12pt" fo:color="#555555"/></style:style>')
put('<style:style style:name="Contents_20_Heading" style:display-name="Contents Heading" style:family="paragraph" style:parent-style-name="Heading" style:class="index"><style:paragraph-properties fo:break-before="page" fo:margin-bottom="0.4cm"/><style:text-properties fo:font-size="20pt"/></style:style>')
put('<style:style style:name="Contents_20_1" style:display-name="Contents 1" style:family="paragraph" style:parent-style-name="Standard" style:class="index"><style:paragraph-properties fo:margin-top="0.18cm"><style:tab-stops><style:tab-stop style:position="17cm" style:type="right" style:leader-style="dotted" style:leader-text="."/></style:tab-stops></style:paragraph-properties><style:text-properties fo:font-weight="bold"/></style:style>')
put('<style:style style:name="Contents_20_2" style:display-name="Contents 2" style:family="paragraph" style:parent-style-name="Standard" style:class="index"><style:paragraph-properties fo:margin-left="0.6cm"><style:tab-stops><style:tab-stop style:position="16.4cm" style:type="right" style:leader-style="dotted" style:leader-text="."/></style:tab-stops></style:paragraph-properties><style:text-properties fo:font-size="9.5pt"/></style:style>')
put('<style:style style:name="Table_20_Contents" style:display-name="Table Contents" style:family="paragraph" style:parent-style-name="Standard" style:class="extra"><style:paragraph-properties fo:margin-top="0.04cm" fo:margin-bottom="0.04cm"/><style:text-properties fo:font-size="9.5pt"/></style:style>')
put('<style:style style:name="Table_20_Heading" style:display-name="Table Heading" style:family="paragraph" style:parent-style-name="Table_20_Contents" style:class="extra"><style:text-properties fo:font-weight="bold" fo:color="#1F3A5F"/></style:style>')
# page A4
auto = root.find('office:automatic-styles', NS)
for pl in auto.findall('style:page-layout', NS):
    pp = pl.find('style:page-layout-properties', NS)
    for k,v in [('page-width','21cm'),('page-height','29.7cm'),('margin-top','1.8cm'),('margin-bottom','1.4cm'),('margin-left','2cm'),('margin-right','2cm')]:
        pp.set(f'{{{NS["fo"]}}}{k}', v)
tree.write(os.path.join(TRAVAIL, 'ref', 'styles.xml'), xml_declaration=True, encoding='utf-8')
# reconstruire l'ODT (mimetype non compressé en premier)
os.makedirs(os.path.join(ICI, 'sortie'), exist_ok=True)
os.chdir(os.path.join(TRAVAIL, 'ref'))
with zipfile.ZipFile(os.path.join(ICI, 'sortie', 'reference.odt'),'w') as z:
    z.write('mimetype', compress_type=zipfile.ZIP_STORED)
    for dp,dn,fn in os.walk('.'):
        for f in fn:
            p=os.path.join(dp,f)[2:]
            if p!='mimetype': z.write(p, compress_type=zipfile.ZIP_DEFLATED)
print('sortie/reference.odt : styles du tutoriel')
