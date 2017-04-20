#ifndef __WS_UTILS_MATH_H__
#define __WS_UTILS_MATH_H__

#include <random>
#include <math.h>
#include <algorithm>

namespace ws
{
	namespace utils
	{
		class Math
		{
		public:
			/************************************************************************/
			/* 产生一个在[0,1)区间的随机浮点数                                        */
			/************************************************************************/
			inline static double						random(){ return randomGenerator() / (0xFFFFFFFF + 1.0); }
			/************************************************************************/
			/* 产生一个在[start,start+range)区间的随机整数                            */
			/************************************************************************/
			inline static unsigned int					random(unsigned int range, unsigned int start = 0){ return (unsigned int)(start + range * random()); }
			/************************************************************************/
			/* 判断一个概率是否触发，默认按万分比计算                                  */
			/************************************************************************/
			inline static bool							isTrigger(unsigned short chance, unsigned short totalChance = 10000){ return chance > random(totalChance); }

		private:
			//std::mt19937::tempering_d
			static std::mt19937							randomGenerator;
		};

		class Vector2D
		{
		public:
			Vector2D(double x_ = 0.0, double y_ = 0.0) :x(x_), y(y_){}
			double				x;
			double				y;

			inline void						zero(){ x = 0.0; y = 0.0; }

			inline double					getLength(){ return sqrt(x * x + y * y); }
			inline Vector2D&				setLength(double value)
			{
				double angle(getAngle());
				x = cos(angle) * value;
				y = sin(angle) * value;
				return *this;
			}

			inline double					getAngle(){ return atan2(y, x); }
			inline Vector2D&				setAngle(double value)
			{
				double length(getLength());
				x = cos(value) * length;
				y = sin(value) * length;
				return *this;
			}

			Vector2D&						normalize()
			{
				double length(getLength());
				if (length == 0.0)
				{
					x = 1;
				}
				else
				{
					x /= length;
					y /= length;
				}
				return *this;
			}

			inline Vector2D&						truncate(double value)
			{
				return setLength(std::min<double>(value, getLength()));
			}

			inline Vector2D&						reverse()
			{
				x = -x;
				y = -y;
				return *this;
			}

			inline operator bool() const { return x == 0 && y == 0; }
			inline Vector2D operator+(const Vector2D& other){ return Vector2D(x + other.x, y + other.y); }
			inline Vector2D operator-(const Vector2D& other){ return Vector2D(x - other.x, y - other.y); }
			inline Vector2D operator*(double value){ return Vector2D(x * value, y * value); }
			inline Vector2D operator/(double value){ return Vector2D(x / value, y / value); }
			inline bool operator==(const Vector2D& other) const { return x == other.x && y == other.y; }
			Vector2D& operator+=(const Vector2D& other)
			{
				x += other.x;
				y += other.y;
				return *this;
			}
			Vector2D& operator-=(const Vector2D& other)
			{
				x -= other.x;
				y -= other.y;
				return *this;
			}

			double dotProduct(const Vector2D& other){ return x * other.x + y * other.y; }
			double crossProduct(const Vector2D& other){ return x * other.y - y * other.x; }
			int sign(const Vector2D& other){ return crossProduct(other) < 0 ? -1 : 1; }
			Vector2D getPerpendicular(){ return Vector2D(-y, x); }
			double distance(const Vector2D& other)
			{
				double dx = other.x - x;
				double dy = other.y - y;
				return sqrt(dx * dx + dy * dy);
			}

			static double angleBetween(Vector2D v1, Vector2D v2)
			{
				v1.normalize();
				v2.normalize();
				return acos(v1.dotProduct(v2));
			}
		};
	}
}

#endif