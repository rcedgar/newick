BINDIR := ../bin
OBJDIR := o
BINPATH := $(BINDIR)/newick

CPPFLAGS := $(CPPFLAGS) -DNDEBUG -pthread

CXX = g++
CXXFLAGS := $(CXXFLAGS) -O3 -fopenmp -ffast-math -msse -mfpmath=sse

UNAME_S := $(shell uname -s)
LDFLAGS := $(LDFLAGS) -O3 -fopenmp -pthread -lpthread
ifeq ($(UNAME_S),Linux)
    LDFLAGS += -static
endif

HDRS = \
  biparter.h \
  biparterx.h \
  clustermaker.h \
  cmds.h \
  consmaker.h \
  consmakerx.h \
  consorder.h \
  countsort.h \
  divconker.h \
  featuretable.h \
  gobuff.h \
  layout.h \
  murmur.h \
  myopts.h \
  myutils.h \
  newick_token.h \
  newicklexer.h \
  newickparser2.h \
  newicktree.h \
  quarts.h \
  sort.h \
  splitter.h \
  supermaker.h \
  svg.h \
  taxer.h \
  tree2.h \
  treen.h \
  treex.h \
  usage.h \

OBJS = \
  $(OBJDIR)/addrootlabel.o \
  $(OBJDIR)/bestfitsubtree.o \
  $(OBJDIR)/biparterx.o \
  $(OBJDIR)/cladeq.o \
  $(OBJDIR)/colors.o \
  $(OBJDIR)/condensex.o \
  $(OBJDIR)/conf2.o \
  $(OBJDIR)/confcmps.o \
  $(OBJDIR)/consensus.o \
  $(OBJDIR)/consensusx.o \
  $(OBJDIR)/consmaker.o \
  $(OBJDIR)/consmakerx.o \
  $(OBJDIR)/consorder.o \
  $(OBJDIR)/correl2.o \
  $(OBJDIR)/deletegroup.o \
  $(OBJDIR)/deleteoutgroup.o \
  $(OBJDIR)/divconker.o \
  $(OBJDIR)/olcs.o \
  $(OBJDIR)/dq.o \
  $(OBJDIR)/featuretablefromtree.o \
  $(OBJDIR)/findgroups.o \
  $(OBJDIR)/getcc.o \
  $(OBJDIR)/getlcas.o \
  $(OBJDIR)/getlcasx.o \
  $(OBJDIR)/lcalabel.o \
  $(OBJDIR)/mono.o \
  $(OBJDIR)/rootbyoutgroup.o \
  $(OBJDIR)/bootq.o \
  $(OBJDIR)/cladogram.o \
  $(OBJDIR)/cluster2.o \
  $(OBJDIR)/clustermaker.o \
  $(OBJDIR)/collapse.o \
  $(OBJDIR)/condense.o \
  $(OBJDIR)/conf.o \
  $(OBJDIR)/deleteleaves.o \
  $(OBJDIR)/diameter.o \
  $(OBJDIR)/drawfs.o \
  $(OBJDIR)/featuretable.o \
  $(OBJDIR)/fixft.o \
  $(OBJDIR)/getlabels.o \
  $(OBJDIR)/intlabel.o \
  $(OBJDIR)/ladderize.o \
  $(OBJDIR)/draw.o \
  $(OBJDIR)/drawf.o \
  $(OBJDIR)/layout.o \
  $(OBJDIR)/murmurhash.o \
  $(OBJDIR)/myutils.o \
  $(OBJDIR)/newicklexer.o \
  $(OBJDIR)/newickparser2.o \
  $(OBJDIR)/newicktree.o \
  $(OBJDIR)/newick_main.o \
  $(OBJDIR)/newick_token.o \
  $(OBJDIR)/biparter.o \
  $(OBJDIR)/quarts.o \
  $(OBJDIR)/randtree.o \
  $(OBJDIR)/relabel.o \
  $(OBJDIR)/relabelacc.o \
  $(OBJDIR)/relabelf.o \
  $(OBJDIR)/relabeln.o \
  $(OBJDIR)/reroot.o \
  $(OBJDIR)/rofo3.o \
  $(OBJDIR)/rofos.o \
  $(OBJDIR)/rootbyhalves.o \
  $(OBJDIR)/rootbyoutgroupx.o \
  $(OBJDIR)/rotate.o \
  $(OBJDIR)/shrink.o \
  $(OBJDIR)/supercluster.o \
  $(OBJDIR)/supermaker.o \
  $(OBJDIR)/tax.o \
  $(OBJDIR)/taxer.o \
  $(OBJDIR)/taxq2.o \
  $(OBJDIR)/taxqx.o \
  $(OBJDIR)/taxtable.o \
  $(OBJDIR)/testdeletesubtree.o \
  $(OBJDIR)/testx.o \
  $(OBJDIR)/topo.o \
  $(OBJDIR)/split.o \
  $(OBJDIR)/stringsfromfile.o \
  $(OBJDIR)/subset.o \
  $(OBJDIR)/subsetnodes.o \
  $(OBJDIR)/subtree.o \
  $(OBJDIR)/svg.o \
  $(OBJDIR)/syncft.o \
  $(OBJDIR)/syncftacc.o \
  $(OBJDIR)/stats.o \
  $(OBJDIR)/taxq.o \
  $(OBJDIR)/tree2.o \
  $(OBJDIR)/treen.o \
  $(OBJDIR)/treesfromdata.o \
  $(OBJDIR)/treex.o \
  $(OBJDIR)/tsv.o \
  $(OBJDIR)/usage.o \

.PHONY: clean

$(BINPATH) : $(BINDIR)/ $(OBJDIR)/ $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $(BINPATH)
	strip -d $(BINPATH)

$(OBJDIR)/ :
	mkdir -p $(OBJDIR)/

$(BINDIR)/ :
	mkdir -p $(BINDIR)/

$(OBJDIR)/%.o : %.cpp $(HDRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)/ $(BINPATH)
