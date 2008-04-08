#ifndef VPGDI_H_INCLUDED
#define VPGDI_H_INCLUDED

#include <windows.h>

namespace VP
{
	class DC
	{
	private:
		struct Data
		{
			Data() : _ref(1), _hdc(NULL), _create(false), _paint(false) {}
			Data(HWND hWnd, LPPAINTSTRUCT lpPaint) : 
				_ref(1), 
				_create(false), 
				_paint(true), 
				_hdc(::BeginPaint(hWnd, lpPaint)),
				_lpPaint(lpPaint), 
				_hWnd(hWnd)
			{
			}
			Data(Data & from, int x, int y) : 
				_ref(1),
				_create(true),
				_paint(false),
				_hdc(::CreateCompatibleDC(from._hdc)),
				_hbmp(x ? ::CreateCompatibleBitmap(from._hdc, x, y) : 0)
			{
				if (x)
					::SelectObject(_hdc, _hbmp);
			}
			Data(HWND hwnd) :
				_ref(1),
				_create(true),
				_paint(false),
				_hdc(::GetDC(hwnd))
			{
			}
			~Data()
			{
				if (_paint && _hdc != NULL)
					::EndPaint(_hWnd, _lpPaint);
				if (_create && _hdc != NULL)
					::DeleteObject(_hdc);
				if (_create && _hbmp != NULL)
					::DeleteObject(_hbmp);
			}
			void AddRef() {++_ref;}
			void Release() {if (!--_ref) delete this;}
			int _ref;
			::HDC _hdc;
			bool _create;
			bool _paint;
			LPPAINTSTRUCT _lpPaint;
			HWND _hWnd;
			HBITMAP _hbmp;
		};
		Data * _data;
	public:
		DC() : _data(0) {}
		explicit DC(HWND hwnd) : _data(new Data(hwnd)) {}
		explicit DC(DC & from, int x, int y) : _data(new Data(*from._data, x, y)) { }
		DC(const DC & from) : _data(from._data)
		{
			if (_data)
				_data->AddRef();
		}
		explicit DC(HWND hWnd, LPPAINTSTRUCT lpPaint) : _data(new Data(hWnd, lpPaint)) {}
		~DC()
		{
			if (_data)
				_data->Release();
		}
		void operator =(DC & from)
		{
			Data * _tmp_data = _data;
			_data = from._data;
			if (_data)
				_data->AddRef();
			if (_tmp_data)
				_tmp_data->Release();
		}
		bool IsCreated()
		{
			return _data != NULL && _data->_create;
		}
		void SelectObject(HGDIOBJ h)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::SelectObject(_data->_hdc, h);
		}
		void getTextExtentPoint(LPCWSTR lpString, LPSIZE lpsz)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::GetTextExtentPoint(_data->_hdc, lpString, wcslen(lpString), lpsz);
		}
		void Polygon(CONST POINT *apt, int cpt)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::Polygon(_data->_hdc, apt, min(cpt, 256));
		}
		void Polyline(CONST POINT *apt, int cpt)
		{
			if (_data != NULL && _data->_hdc != NULL)
			{
				while (cpt > 1)
				{
					int cpt1 = min(cpt, 256);
					::Polyline(_data->_hdc, apt, cpt1);
					apt += (cpt1 - 1);
					cpt -= (cpt1 - 1);
				}
			}
		}
		void SetTextColor(COLORREF color)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::SetTextColor(_data->_hdc, color);
		}
		void ExtTextOut(int x, int y, UINT options, CONST RECT * lprect, LPCWSTR lpString, CONST INT * lpDx)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::ExtTextOut(_data->_hdc, x, y, options, lprect, lpString, wcslen(lpString), lpDx);
		}
		void RoundRect(int left, int top, int right, int bottom, int width, int height)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::RoundRect(_data->_hdc, left, top, right, bottom, width, height);
		}
		void FillRect(CONST RECT *lprc, HBRUSH hbr)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::FillRect(_data->_hdc, lprc, hbr);
		}
		void SetBkMode(int mode)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::SetBkMode(_data->_hdc, mode);
		}
		void drawIcon(int X, int Y, HICON hIcon)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::DrawIcon(_data->_hdc, X, Y, hIcon);
		}
		void Ellipse(int left, int top, int right, int bottom)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::Ellipse(_data->_hdc, left, top, right, bottom);
		}
		void Rectangle(int left, int top, int right, int bottom)
		{
			if (_data != NULL && _data->_hdc != NULL)
				::Rectangle(_data->_hdc, left, top, right, bottom);
		}
		void BitBlt(int x, int y, int cx, int cy, DC & hdcSrc, int x1, int y1, DWORD rop)
		{
			if (_data != NULL && _data->_hdc != NULL && hdcSrc._data != NULL && hdcSrc._data->_hdc != NULL)
				::BitBlt(_data->_hdc, x, y, cx, cy, hdcSrc._data->_hdc, x1, y1, rop);
		}
#if defined(SMARTPHONE) && defined(AC_SRC_OVER)
		void AlphaBlend(int x, int y, int cx, int cy, DC & hdcSrc, int x1, int y1, int cx1, int cy1, BLENDFUNCTION bf)
		{
			if (_data != NULL && _data->_hdc != NULL && hdcSrc._data != NULL && hdcSrc._data->_hdc != NULL)
				::AlphaBlend(_data->_hdc, x, y, cx, cy, hdcSrc._data->_hdc, x1, y1, cx1, cy1, bf);			
		}
#endif // SMARTPHONE
		HDC Get() {return _data->_hdc;}
	};

	class Buffer
	{
		DC _context;
		int _x;
		int _y;
	public:
		VP::DC & GetContext(DC & original, int x, int y)
		{
			if (_context.IsCreated())
			{
				if (x <= _x && y <= _y)
					return _context;
				x = max(x, _x);
				y = max(y, _y);
			}
			_context = DC(original, x, y);
			return _context;
		}
	};
}

#endif // VPGDI_H_INCLUDED
