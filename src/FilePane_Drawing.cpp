
#include "FilePane_Common.h"

void DrawPointsAsLinePairsCenteredInBox(Graphics *g, Pen *pen, PointF *points, int count, float rcx, float rcy, float rcw, float rch)
{
    Center(points, count, rcx, rcy, rcw, rch);
    g->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
    for (int i = 0; i < count;i+=2) {
        g->DrawLine(pen, points[i], points[i+1]);
    }
}


float GetHeight(PointF *points, int count)
{
    float min = 1e7f;
    float max = 0;
    for(int i = 0; i < count; i++) {
        if (points[i].Y > max) {
            max = points[i].Y;
        }
        if (points[i].Y < min) {
            min =  points[i].Y;
        }
    }
    return max - min;
}

float GetWidth(PointF *points, int count)
{
    float min = 1e7f;
    float max = 0;
    for(int i = 0; i < count; i++) {
        if (points[i].X > max) {
            max = points[i].X;
        }
        if (points[i].X < min) {
            min = points[i].X;
        }
    }
    return max - min;
}

void Scale(PointF *points, int count, float scale)
{
    for(int i = 0; i < count; i++)
    {
        points[i].X *= scale;
        points[i].Y *= scale;
    }
}

void Translate(PointF *points, int count, float x, float y)
{
    for(int i = 0; i < count; i++)
    {
        points[i].X += x;
        points[i].Y += y;
    }
}

void Center(PointF *points, int count, float rcx, float rcy, float rcw, float rch)
{
    float w = GetWidth(points, count);
    float h = GetHeight(points, count);
    Translate(points, count, rcx + (rcw - w)/2.0f, rcy + (rch - h)/2.0f);
}

void matmul(PointF *point, mat2x2 *mat)
{
    float x = point->X;
    float y = point->Y;
    point->X = x*mat->col1[0] + y*mat->col1[1];
    point->Y = x*mat->col2[0] + y*mat->col2[1];
}

void Rotate90(PointF *points, int count)
{
    mat2x2 mat = { {0.0f, 1.0f}, {-1.0f, 0.0f} };
    for (int i = 0; i < count; i++) {
        matmul(&points[i], &mat);
    }
}

void DrawExplorerFrame(Pane *pane, HDC hdc, HBRUSH brush)
{
    RECT left, top, right, bottom;
    
    left.left = pane->rc.left - FRAME_WIDTH;
    left.top = pane->rc.top - FRAME_WIDTH;
    left.right = pane->rc.left;
    left.bottom = pane->rc.bottom + FRAME_WIDTH;

    top.left = pane->rc.left - FRAME_WIDTH;
    top.top = pane->rc.top - FRAME_WIDTH;
    top.right = pane->rc.right + FRAME_WIDTH;
    top.bottom = pane->rc.top;

    right.left = pane->rc.right;
    right.top = pane->rc.top - FRAME_WIDTH;
    right.right = pane->rc.right + FRAME_WIDTH;
    right.bottom = pane->rc.bottom + FRAME_WIDTH;

    bottom.left = pane->rc.left - FRAME_WIDTH;
    bottom.top = pane->rc.bottom;
    bottom.right = pane->rc.right + FRAME_WIDTH;
    bottom.bottom = pane->rc.bottom + FRAME_WIDTH;

    FillRect(hdc, &left, brush);
    FillRect(hdc, &top, brush);
    FillRect(hdc, &right, brush);
    FillRect(hdc, &bottom, brush);
}


