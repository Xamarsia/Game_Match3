#pragma once
namespace Common
{
    int getColumn(const int index, const int rowsCount, const int columnsCount) {
        if(index > ((rowsCount*columnsCount)-1) || index < 0) {
            return -1;
        }
        return index % columnsCount;
    }

    int getRow(const int index, const int rowsCount, const int columnsCount) {
        if(index > ((rowsCount*columnsCount)-1) || index < 0) {
            return -1;
        }
        return (index / columnsCount) | 0;
    }
}
