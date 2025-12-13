class solverUtilities {
public:
    static std::pair<int, int> makeRandomMove(std::vector<std::pair<int, int>> availableMoves) {
        int index = rand() % availableMoves.size();
        return availableMoves[index];
    }

};