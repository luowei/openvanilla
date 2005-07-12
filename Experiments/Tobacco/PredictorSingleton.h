#ifndef PREDICTORSINGLETON_H
#define PREDICTORSINGLETON_H

#include <vector>
#include <string>

#include "DictionarySingleton.h"
#include "Candidate.h"
#include "Token.h"

using namespace std;

class PredictorSingleton
{
public:
    string composedString;
	vector< vector<string> > vectorOfCharacterVector;
	vector<Token> tokenVector;
	vector<Candidate> candidateVector;
	vector<int> candidatePositionVector;

	static PredictorSingleton* getInstance(
	   const char* dbFilePath, const char* inputMethodId)
	{
		if(itsInstance == NULL)
			itsInstance = new PredictorSingleton(dbFilePath, inputMethodId);

		return itsInstance;
	}

	void lostInstance();
    
	void setTokenVector(string currentSequence, int position);
	void setCandidateVector(int position);
	void setSelectedCandidate(int position, int selectedCandidateIndex);

	void removeWord(int position, bool delFlag);
	void clearAll();

protected:
	void addCandidates(string characters, int head);
    void setTokenVectorByBigram();
    void setComposedString();

	PredictorSingleton(const char* dbFilePath, const char* inputMethodId);
	~PredictorSingleton();

private:
	static PredictorSingleton* itsInstance;
	DictionarySingleton* dictionary;
};

#endif //#ifndef PREDICTORSINGLETON_H