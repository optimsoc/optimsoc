import opensocdebug
import sys
from optparse import OptionParser

parser = OptionParser(usage="usage: %prog [options] <elffile>")
parser.add_option("--verify-memload",
                  action="store_true", dest="verify", default=False,
                  help="verify loaded memory")
parser.add_option("--no-ctm",
                  action="store_false", dest="ctm", default=True,
                  help="don't use CTM")
parser.add_option("--no-stm",
                  action="store_false", dest="stm", default=True,
                  help="don't use STM")

(options, args) = parser.parse_args()

if len(args) != 1:
        parser.error("missing ELF file")

elffile = args[0]

osd = opensocdebug.Session()

osd.reset(halt=True)

if options.stm:
    for m in osd.get_modules("STM"):
        m.log("stm{:03d}.log".format(m.get_id()))

if options.ctm:
    for m in osd.get_modules("CTM"):
        m.log("ctm{:03d}.log".format(m.get_id()), elffile)

for m in osd.get_modules("MAM"):
    m.loadelf(elffile, options.verify)

osd.start()
osd.wait(10)
