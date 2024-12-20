CXX = g++
CXXFLAGS = -std=c++14 -O3 -pthread $(MACRO)
MPICXX = mpic++

COMMON= core/utils.h core/cxxopts.h core/get_time.h
SERIAL= lcs_serial 
PARALLEL= lcs_parallel_columns_1 lcs_parallel_columns_2 lcs_parallel_diagonal
MPI= lcs_mpi
ALL= $(SERIAL) $(PARALLEL) $(MPI)


all : $(ALL)

lcs_mpi : lcs_mpi.cpp $(COMMON)
	$(MPICXX) $(CXXFLAGS) -o $@ $<

% : %.cpp $(COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY : clean

clean :
	rm -f *.o *.obj $(ALL)
