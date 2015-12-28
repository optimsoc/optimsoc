#!/usr/bin/env python

from Templite.Templite import Templite
import sys

class LISNoCGenerator:
    xdim = 2
    ydim = 2
    vc = 0

    def __init__(self,xdim,ydim,vc):
        self.xdim = int(xdim)
        self.ydim = int(ydim)
        self.vc = int(vc)

    def generate(self,module=None):
        if module==None:
            if self.vc > 0:
                module = "lisnoc_mesh%dx%d_vc%d" % (self.xdim,self.ydim,self.vc)
            else:
                module = "lisnoc_mesh%dx%d" % (self.xdim,self.ydim)
        out = open("%s.v"%module,"w")

        # we generate the lookups here
        lookup = {};
        for xs in range(0,self.xdim):
            for ys in range(0,self.ydim):
                l = []
                for xd in range(0,self.xdim):
                    for yd in range (0,self.ydim):
                        if xd==xs and yd==ys:
                            l.append("`SELECT_LOCAL")
                        elif xd==xs:
                            if yd<ys:
                                l.append("`SELECT_WEST")
                            else:
                                l.append("`SELECT_EAST")
                        else:
                            if xd<xs:
                                l.append("`SELECT_NORTH")
                            else:
                                l.append("`SELECT_SOUTH")
                lookup[xs*self.ydim+ys] = "{" + str(",").join(l) + "}"

        template = Templite(open("lisnoc_mesh.vt").read())
        if self.vc==0:
            self.vc=1
        out.write(template.render(modulename=module,vc=self.vc,xdim=self.xdim,ydim=self.ydim,lookup=lookup))

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "Usage: lisnoc_generate.py <xdim> <ydim> [<vc>]"
        exit(1)

    xdim = sys.argv[1]
    ydim = sys.argv[2]
    if len(sys.argv)>3:
        vc = sys.argv[3]
    else:
        vc = 0

    generator = LISNoCGenerator(xdim,ydim,vc)
    generator.generate()
