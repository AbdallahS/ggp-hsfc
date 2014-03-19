all: HSFC

HSFC: hsfcAPI.o Example.cpp
	g++ hsfcAPI.o hsfcEngine.o hsfcGrinder.o hsfcState.o hsfcSchema.o hsfcGDL.o hsfcLexicon.o Example.cpp -o Example.exe

hsfcAPI.o: hsfcEngine.o hsfcAPI.cpp  
	g++ -c hsfcAPI.cpp 

hsfcEngine.o: hsfcGrinder.o hsfcEngine.cpp  
	g++ -c hsfcEngine.cpp 

hsfcGrinder.o: hsfcState.o hsfcGrinder.cpp  
	g++ -c hsfcGrinder.cpp 

hsfcState.o: hsfcSchema.o hsfcState.cpp  
	g++ -c hsfcState.cpp 

hsfcSchema.o: hsfcGDL.o hsfcSchema.cpp  
	g++ -c hsfcSchema.cpp 

hsfcGDL.o: hsfcLexicon.o hsfcGDL.cpp  
	g++ -c hsfcGDL.cpp 

hsfcLexicon.o: hsfcLexicon.cpp  
	g++ -c hsfcLexicon.cpp 



