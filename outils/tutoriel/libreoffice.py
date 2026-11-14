# Ouvre le document dans LibreOffice : met à jour le sommaire, met en forme les tableaux,
# enregistre le .odt final et exporte le PDF.
# Usage : python3 libreoffice.py entree.odt sortie.odt sortie.pdf
import uno, subprocess, time, sys, os
from com.sun.star.beans import PropertyValue
from com.sun.star.table import BorderLine2
def P(n,v):
    p=PropertyValue(); p.Name=n; p.Value=v; return p
src=os.path.abspath(sys.argv[1]); odt=os.path.abspath(sys.argv[2]); pdf=os.path.abspath(sys.argv[3])
proc=subprocess.Popen(['soffice','--headless','--invisible','--norestore','--accept=socket,host=localhost,port=2002;urp;'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
ctx=None
for _ in range(60):
    try:
        local=uno.getComponentContext()
        resolver=local.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver',local)
        ctx=resolver.resolve('uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext'); break
    except Exception: time.sleep(1)
desktop=ctx.ServiceManager.createInstanceWithContext('com.sun.star.frame.Desktop',ctx)
doc=desktop.loadComponentFromURL(uno.systemPathToFileUrl(src),'_blank',0,(P('Hidden',True),))
# Tableaux : bordures fines, en-tête coloré
def line(w,col):
    b=BorderLine2(); b.OuterLineWidth=w; b.Color=col; return b
tables=doc.getTextTables()
for i in range(tables.getCount()):
    t=tables.getByIndex(i)
    tb=t.TableBorder2
    for side in ('TopLine','BottomLine','LeftLine','RightLine','HorizontalLine','VerticalLine'):
        setattr(tb,side,line(18,0xA9B8C9))
    tb.IsTopLineValid=tb.IsBottomLineValid=tb.IsLeftLineValid=tb.IsRightLineValid=tb.IsHorizontalLineValid=tb.IsVerticalLineValid=True
    t.TableBorder2=tb
    rows=t.getRows()
    names=t.getCellNames()
    for n in names:
        c=t.getCellByName(n)
        c.LeftBorderDistance=c.RightBorderDistance=120
        c.TopBorderDistance=c.BottomBorderDistance=40
    # première ligne : fond bleu clair
    cols=t.getColumns().getCount()
    for n in names:
        if n[-1]=='1' and n[:-1].isalpha():
            t.getCellByName(n).BackColor=0xDCE6F1
    t.RepeatHeadline=True
# Sommaire et numéros de page
idx=doc.getDocumentIndexes()
for i in range(idx.getCount()): idx.getByIndex(i).update()
doc.refresh()
for i in range(idx.getCount()): idx.getByIndex(i).update()
doc.storeToURL(uno.systemPathToFileUrl(odt),(P('FilterName','writer8'),))
doc.storeToURL(uno.systemPathToFileUrl(pdf),(P('FilterName','writer_pdf_Export'),P('FilterData',uno.Any('[]com.sun.star.beans.PropertyValue',tuple([P('ExportBookmarks',True),P('UseTaggedPDF',True)])))))
doc.close(True)
try: desktop.terminate()
except Exception: pass
proc.wait(timeout=30)
print('ok')
