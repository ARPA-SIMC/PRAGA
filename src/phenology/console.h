#ifndef CONSOLE_H
#define CONSOLE_H

    #include <iostream>

    class Console
    {
    private:
        FILE* m_stream;

    public:
        Console() : m_stream(nullptr) {}

        void Open(const char* filename, const char* mode);
        void Close();
        void Read(char* message);
        void Write(const char* message);
        int EndOfFile();
        int Show(const char* message);

        FILE* Stream() { return m_stream; }
    };


    // functions
    void login(const char* log, Console& console);
    void logout(Console& console);

#endif
