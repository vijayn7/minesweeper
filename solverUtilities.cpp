class solverUtilities {
public:

    static std::pair<int, int> makeRandomMove(std::vector<std::pair<int, int>> availableMoves) {
        int index = getRandomInt(0, availableMoves.size() - 1);
        return availableMoves[index];
    }

    static int getRandomInt(int min, int max) {
        return rand() % (max - min + 1) + min;
    }

};