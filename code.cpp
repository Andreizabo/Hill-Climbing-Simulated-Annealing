#include <vector>
#include <iostream>
#include <thread>
#include <cmath>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iomanip>

#define PI 3.14159265358979323846
#define M 10

using namespace std;

double a = -500, b = 500; //margins
int prec = 5; //precision
unsigned int n; //dimensions
int l; //length

//FUNCTIONS

double deJong(std::vector<double>& v) {
	double result = 0;
	for (double d : v) {
		result += (d * d);
	}
	return result;
}

double schwefel(std::vector<double>& v) {
	double result = 0;
	for (double d : v) {
		result += (d * sin(sqrt(abs(d))) * -1);
	}
	return result;
}

double rastrigin(std::vector<double>& v) {
	double result = 10 * v.size();
	for (double d : v) {
		result += ((d * d) - (10 * cos(2 * PI * d)));
	}
	return result;
}

double michalewicz(std::vector<double>& v) {
	double result = 0;
	int size = v.size();
	for (int i = 1; i <= size; ++i) {
		result += (sin(v[i - 1]) * pow(sin((i * v[i - 1] * v[i - 1]) / PI), M * 2));
	}
	result *= -1;
	return result;
}

//END OF FUNCTIONS

int length(double a, double b, int prec)
{
	return ceil(log((b - a) * pow(10, prec)) / log(2));
}

double decodeDimension(vector<char>::iterator itStart, vector<char>::iterator itEnd, int l, double a, double b)
{
    unsigned long bi = 0;
    for (auto i = itStart; i != itEnd; ++i)
    {
        bi *= 2;
        bi += *i;
    }

    double s = bi / (pow(2, l) - 1);
    return s * (b - a) + a;

}

vector<double> decode(vector<char>& bits, int l, unsigned int n, double a, double b)
{
    vector<double> ret;
    vector<char>::iterator itStart, itEnd;
    for (int i = 0; i < n; ++i) {
        itStart = bits.begin() + i * l;
        itEnd = itStart + l;

        double x = decodeDimension(itStart, itEnd, l, a, b);
        ret.push_back(x);
    }
    return ret;
}

double hillClimbingBest(vector<char>& bits, unsigned int n, int l, int bitsSize, double a, double b, double (*func)(vector<double> &))
{
	auto decoded = decode(bits, l, n, a, b);
	double min = func(decoded);
	bool local = false;
	int k = 0;
	do {
		local = false;
		double initial = min;
		for (int i = 0; i < bitsSize; ++i)
		{
			bits[i] = !bits[i];

			decoded = decode(bits, l, n, a, b);
			double val = func(decoded);
			if (val < min)
			{
				min = val;
				k = i;
			}
			bits[i] = !bits[i];
		}
		if (min == initial) {
			local = true;
		}
		else {
			bits[k] = !bits[k];
		}
	} while (!local);

	return min;
}

double hillClimbingFirst(vector<char>& bits, unsigned int n, int l, int bitsSize, double a, double b, double (*func)(vector<double> &))
{
	auto decoded = decode(bits, l, n, a, b);
	double min = func(decoded);
	bool local = false;
	do {
		local = false;
		double initial = min;
		for (int i = 0; i < bitsSize; ++i)
		{
			bits[i] = !bits[i];

			decoded = decode(bits, l, n, a, b);
			double val = func(decoded);
			if (val < min)
			{
				min = val;
				break;
			}
			bits[i] = !bits[i];
		}
		if (min == initial) {
			local = true;
		}
	} while (!local);

	return min;
}

void simulatedAnnealing(unsigned int n, int l, int bitsSize, double a, double b, double (*func)(vector<double>&), ofstream &f) {
	clock_t start, end;
	start = clock();
	vector<char> bits;
	for (int i = 0; i < bitsSize; ++i) {
		bits.push_back(rand() % 2);
	}
	auto decoded = decode(bits, l, n, a, b);
	double min = func(decoded);
	
	double temperature = 100;
	
	while (temperature > 0.000000001) {
		for (int i = 0; i < 10000; ++i) {
			int random = rand() % bitsSize;

			bits[random] = !bits[random];

			decoded = decode(bits, l, n, a, b);
			double val = func(decoded);

			if (val < min) {
				min = val;
			}
			else if ((double)(rand() % 10000000) / (double)10000000 < exp(-1 * abs(val - min) / temperature)) {
				min = val;
			}
			else {
				bits[random] = !bits[random];
			}
		}
		temperature *= 0.9942601;
	}
	decoded = decode(bits, l, n, a, b);
	f << "\nGlobal minimum = " << min << " found.\nPoint: (";
	for (int i = 0; i < decoded.size(); ++i) {
		f << decoded[i];
		if (i < decoded.size() - 1) {
			f << ", ";
		}
		else {
			f << ")";
		}
	}
	end = clock();
	f << "\nThread took " << (double)((double)(end - start) / (double)CLOCKS_PER_SEC) << " seconds.\n";
}

int getIntId(thread::id id) {
	stringstream buffer;
	buffer << id;
	return stoull(buffer.str());
}

void iteratedHillClimbingBest(int size, double (*func)(vector<double> &), clock_t beginning, const char* function) {
	//Creates file and applies HillClimbingBest 10000 times
	clock_t start, end;
	start = clock();
	int id = getIntId(this_thread::get_id());
	srand(id * clock());
	char* fileName = new char[256];
	strcpy(fileName, function);
	strcat(fileName, "/hc_best/");
	strcat(fileName, to_string(size).c_str());
	strcat(fileName, "_Thread_");
	strcat(fileName, to_string(id).c_str());
	strcat(fileName, "_");
	strcat(fileName, to_string(clock() - beginning).c_str());
	strcat(fileName, ".txt");
	ofstream aux(fileName);
	aux << "This is the thread with id = " << id << ", executing for size = " << size << '\n';
	int L = l * size;
	double min = 1000000;
	int kMin = 0;
	vector<char> bits;
	vector<double> bitsMin;
	for (int t = 0; t < 10000; ++t) {
		bits.clear();
		for (int i = 0; i < L; ++i) {
			bits.push_back(rand() % 2);
		}
		double ax = hillClimbingBest(bits, size, l, L, a, b, func);
		if (ax < min) {
			min = ax;
			kMin = t;
			bitsMin = decode(bits, l, size, a, b);
		}
	}
	aux << "\nGlobal minimum = " << min << " found at " << kMin << " iteration.\nPoint: (";
	for (int i = 0; i < bitsMin.size(); ++i) {
		aux << bitsMin[i];
		if (i < bitsMin.size() - 1) {
			aux << ", ";
		}
		else {
			aux << ")";
		}
	}
	end = clock();
	aux << "\nThread with id = " << id << " took " << (double)((double)(end - start) / (double)CLOCKS_PER_SEC) << " seconds.\n";
	aux.close();
}	

void iteratedHillClimbingFirst(int size, double (*func)(vector<double> &), clock_t beginning, const char* function) {
	//Creates file and applies HillClimbingFirst 10000 times
	clock_t start, end;
	start = clock();
	int id = getIntId(this_thread::get_id());
	srand(id * clock());
	char* fileName = new char[256];
	strcpy(fileName, function);
	strcat(fileName, "/hc_first/");
	strcat(fileName, to_string(size).c_str());
	strcat(fileName, "_Thread_");
	strcat(fileName, to_string(id).c_str());
	strcat(fileName, "_");
	strcat(fileName, to_string(clock() - beginning).c_str());
	strcat(fileName, ".txt");
	ofstream aux(fileName);
	aux << "This is the thread with id = " << id << ", executing for size = " << size << '\n';
	int L = l * size;
	double min = 1000000;
	int kMin = 0;
	vector<char> bits;
	vector<double> bitsMin;
	for (int t = 0; t < 10000; ++t) {
		bits.clear();
		for (int i = 0; i < L; ++i) {
			bits.push_back(rand() % 2);
		}
		double ax = hillClimbingFirst(bits, size, l, L, a, b, func);
		if (ax < min) {
			min = ax;
			kMin = t;
			bitsMin = decode(bits, l, size, a, b);
		}
	}
	aux << "\nGlobal minimum = " << min << " found at " << kMin << " iteration.\nPoint: (";
	for (int i = 0; i < bitsMin.size(); ++i) {
		aux << bitsMin[i];
		if (i < bitsMin.size() - 1) {
			aux << ", ";
		}
		else {
			aux << ")";
		}
	}
	end = clock();
	aux << "\nThread with id = " << id << " took " << (double)((double)(end - start) / (double)CLOCKS_PER_SEC) << " seconds.\n";
	aux.close();
}

void mainFunctionSimulatedAnnealing(int size, double (*func)(vector<double>&), clock_t beginning, const char* function) {
	//Creates the file and applies SimulatedAnnealing
	int id = getIntId(this_thread::get_id());
	srand(id * clock());
	char* fileName = new char[256];
	strcpy(fileName, function);
	strcat(fileName, "/sa/");
	strcat(fileName, to_string(size).c_str());
	strcat(fileName, "_Thread_");
	strcat(fileName, to_string(id).c_str());
	strcat(fileName, "_");
	strcat(fileName, to_string(clock() - beginning).c_str());
	strcat(fileName, ".txt");
	ofstream aux(fileName);
	aux << "This is the thread with id = " << id << ", executing for size = " << size << '\n';
	int L = l * size;
	simulatedAnnealing(size, l, L, a, b, func, aux);
	aux.close();
}

void solve(int aArg, int bArg, int precisionArg, double (*func)(vector<double> &), const char* function) {
	a = aArg;
	b = bArg;
	prec = precisionArg;
	l = length(a, b, prec);

	clock_t start, end;
	start = clock();
	cout << "\nStarting analysis of " << function << "\n";
	cout << "Starting Hill Climbing Best\n";
	cout << "Starting threads for size 5\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(iteratedHillClimbingBest, 5, func, start, function);
		thread t2(iteratedHillClimbingBest, 5, func, start, function);
		thread t3(iteratedHillClimbingBest, 5, func, start, function);
		thread t4(iteratedHillClimbingBest, 5, func, start, function);
		thread t5(iteratedHillClimbingBest, 5, func, start, function);
		thread t6(iteratedHillClimbingBest, 5, func, start, function);
		thread t7(iteratedHillClimbingBest, 5, func, start, function);
		thread t8(iteratedHillClimbingBest, 5, func, start, function);
		thread t9(iteratedHillClimbingBest, 5, func, start, function);
		thread t10(iteratedHillClimbingBest, 5, func, start, function);
		thread t11(iteratedHillClimbingBest, 5, func, start, function);
		thread t12(iteratedHillClimbingBest, 5, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 5\n";
	}
	cout << "Starting threads for size 10\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(iteratedHillClimbingBest, 10, func, start, function);
		thread t2(iteratedHillClimbingBest, 10, func, start, function);
		thread t3(iteratedHillClimbingBest, 10, func, start, function);
		thread t4(iteratedHillClimbingBest, 10, func, start, function);
		thread t5(iteratedHillClimbingBest, 10, func, start, function);
		thread t6(iteratedHillClimbingBest, 10, func, start, function);
		thread t7(iteratedHillClimbingBest, 10, func, start, function);
		thread t8(iteratedHillClimbingBest, 10, func, start, function);
		thread t9(iteratedHillClimbingBest, 10, func, start, function);
		thread t10(iteratedHillClimbingBest, 10, func, start, function);
		thread t11(iteratedHillClimbingBest, 10, func, start, function);
		thread t12(iteratedHillClimbingBest, 10, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 10\n";
	}
	cout << "Starting threads for size 30\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(iteratedHillClimbingBest, 30, func, start, function);
		thread t2(iteratedHillClimbingBest, 30, func, start, function);
		thread t3(iteratedHillClimbingBest, 30, func, start, function);
		thread t4(iteratedHillClimbingBest, 30, func, start, function);
		thread t5(iteratedHillClimbingBest, 30, func, start, function);
		thread t6(iteratedHillClimbingBest, 30, func, start, function);
		thread t7(iteratedHillClimbingBest, 30, func, start, function);
		thread t8(iteratedHillClimbingBest, 30, func, start, function);
		thread t9(iteratedHillClimbingBest, 30, func, start, function);
		thread t10(iteratedHillClimbingBest, 30, func, start, function);
		thread t11(iteratedHillClimbingBest, 30, func, start, function);
		thread t12(iteratedHillClimbingBest, 30, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 30\n";
	}
	end = clock();
	cout << function << " with HCB took " << (double)((double)(end - start) / (double)(CLOCKS_PER_SEC)) << " seconds.\n";

	//HERE ENDS BEST
	//HERE STARTS FIRST

	start = clock();
	cout << "Starting Hill Climbing First\n";
	cout << "Starting threads for size 5\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(iteratedHillClimbingFirst, 5, func, start, function);
		thread t2(iteratedHillClimbingFirst, 5, func, start, function);
		thread t3(iteratedHillClimbingFirst, 5, func, start, function);
		thread t4(iteratedHillClimbingFirst, 5, func, start, function);
		thread t5(iteratedHillClimbingFirst, 5, func, start, function);
		thread t6(iteratedHillClimbingFirst, 5, func, start, function);
		thread t7(iteratedHillClimbingFirst, 5, func, start, function);
		thread t8(iteratedHillClimbingFirst, 5, func, start, function);
		thread t9(iteratedHillClimbingFirst, 5, func, start, function);
		thread t10(iteratedHillClimbingFirst, 5, func, start, function);
		thread t11(iteratedHillClimbingFirst, 5, func, start, function);
		thread t12(iteratedHillClimbingFirst, 5, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 5\n";
	}
	cout << "Starting threads for size 10\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(iteratedHillClimbingFirst, 10, func, start, function);
		thread t2(iteratedHillClimbingFirst, 10, func, start, function);
		thread t3(iteratedHillClimbingFirst, 10, func, start, function);
		thread t4(iteratedHillClimbingFirst, 10, func, start, function);
		thread t5(iteratedHillClimbingFirst, 10, func, start, function);
		thread t6(iteratedHillClimbingFirst, 10, func, start, function);
		thread t7(iteratedHillClimbingFirst, 10, func, start, function);
		thread t8(iteratedHillClimbingFirst, 10, func, start, function);
		thread t9(iteratedHillClimbingFirst, 10, func, start, function);
		thread t10(iteratedHillClimbingFirst, 10, func, start, function);
		thread t11(iteratedHillClimbingFirst, 10, func, start, function);
		thread t12(iteratedHillClimbingFirst, 10, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 10\n";
	}
	cout << "Starting threads for size 30\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(iteratedHillClimbingFirst, 30, func, start, function);
		thread t2(iteratedHillClimbingFirst, 30, func, start, function);
		thread t3(iteratedHillClimbingFirst, 30, func, start, function);
		thread t4(iteratedHillClimbingFirst, 30, func, start, function);
		thread t5(iteratedHillClimbingFirst, 30, func, start, function);
		thread t6(iteratedHillClimbingFirst, 30, func, start, function);
		thread t7(iteratedHillClimbingFirst, 30, func, start, function);
		thread t8(iteratedHillClimbingFirst, 30, func, start, function);
		thread t9(iteratedHillClimbingFirst, 30, func, start, function);
		thread t10(iteratedHillClimbingFirst, 30, func, start, function);
		thread t11(iteratedHillClimbingFirst, 30, func, start, function);
		thread t12(iteratedHillClimbingFirst, 30, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 30\n";
	}
	end = clock();
	cout << function << " with HCF took " << (double)((double)(end - start) / (double)(CLOCKS_PER_SEC)) << " seconds.\n";

	//HERE ENDS FIRST
	//HERE STARTS SA

	start = clock();
	cout << "Starting Simulated Annealing\n";
	cout << "Starting threads for size 5\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t2(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t3(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t4(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t5(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t6(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t7(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t8(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t9(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t10(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t11(mainFunctionSimulatedAnnealing, 5, func, start, function);
		thread t12(mainFunctionSimulatedAnnealing, 5, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 5\n";
	}
	cout << "Starting threads for size 10\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t2(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t3(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t4(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t5(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t6(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t7(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t8(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t9(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t10(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t11(mainFunctionSimulatedAnnealing, 10, func, start, function);
		thread t12(mainFunctionSimulatedAnnealing, 10, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 10\n";
	}
	cout << "Starting threads for size 30\n";
	for (int i = 0; i < 3; ++i) { //running 36 times
		thread t1(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t2(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t3(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t4(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t5(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t6(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t7(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t8(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t9(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t10(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t11(mainFunctionSimulatedAnnealing, 30, func, start, function);
		thread t12(mainFunctionSimulatedAnnealing, 30, func, start, function);

		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
		t9.join();
		t10.join();
		t11.join();
		t12.join();

		cout << "Finished iteration " << i << " for size 30\n";
	}
	end = clock();
	cout << function << " with SA took " << (double)((double)(end - start) / (double)(CLOCKS_PER_SEC)) << " seconds.\n";
}

int main() {
	//DEJONG
	solve(-5.12, 5.12, 5, deJong, "DeJong");
	//SCHWEFEL
	solve(-500, 500, 5, schwefel, "Schwefel");
	//Rastrigin
	solve(-5.12, 5.12, 5, rastrigin, "Rastrigin");
	//Michalewicz
	solve(0, PI, 5, michalewicz, "Michalewicz");

	return 0;
}