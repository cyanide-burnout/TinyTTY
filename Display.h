// Copyright 2012 Artem Prilutskiy

#ifndef DISPLAY_H
#define DISPLAY_H

class Display
{
  public:
    virtual void clear() = 0;
    virtual void newLine() = 0;
    virtual void backSpace() = 0;
    virtual void print(uint32_t code) = 0;

    virtual void setCursor(int y, int x) = 0;

    virtual void cursor() = 0;
    virtual void noCursor() = 0;

    virtual int getColumn() = 0;
    virtual int getRow() = 0;

    virtual int getWidth() = 0;
    virtual int getHeight() = 0;
};

#endif
