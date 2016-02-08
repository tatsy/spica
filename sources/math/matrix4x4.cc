#define SPICA_API_EXPORT
#include "matrix4x4.h"

#include <sstream>
#include <iomanip>
#include <cstring>

namespace spica {
    
    Matrix4x4::Matrix4x4() {
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0;
        m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] = m[1][3] = 
            m[2][0] = m[2][1] = m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.0;
    }

    Matrix4x4::Matrix4x4(const Matrix4x4& mat) {
        this->operator=(mat);
    }

    Matrix4x4::Matrix4x4(double mat[4][4]) {
        memcpy(m, mat, sizeof(m));
    }

    Matrix4x4::Matrix4x4(double t00, double t01, double t02, double t03,
                         double t10, double t11, double t12, double t13,
                         double t20, double t21, double t22, double t23,
                         double t30, double t31, double t32, double t33) {
        m[0][0] = t00;
        m[0][1] = t01;
        m[0][2] = t02;
        m[0][3] = t03;
        m[1][0] = t10;
        m[1][1] = t11;
        m[1][2] = t12;
        m[1][3] = t13;
        m[2][0] = t20;
        m[2][1] = t21;
        m[2][2] = t22;
        m[2][3] = t23;
        m[3][0] = t30;
        m[3][1] = t31;
        m[3][2] = t32;
        m[3][3] = t33;
    }

    Matrix4x4 Matrix4x4::transposed() const {
        return Matrix4x4(m[0][0], m[1][0], m[2][0], m[3][0],
                         m[0][1], m[1][1], m[2][1], m[3][1],
                         m[0][2], m[1][2], m[2][2], m[3][2],
                         m[0][3], m[1][3], m[2][3], m[3][3]);
    }

    Matrix4x4 Matrix4x4::inverted() const {
        int indxc[4], indxr[4];
        int ipiv[4] = { 0, 0, 0, 0 };

        double minv[4][4];
        memcpy(minv, m, sizeof(minv));

        for (int i = 0; i < 4; i++) {
            int irow = 0, icol = 0;
            double big = 0.0;

            // Choose pivot
            for (int j = 0; j < 4; j++) {
                if (ipiv[j] != 1) {
                    for (int k = 0; k < 4; k++) {
                        if (ipiv[k] == 0) {
                            if (std::abs(minv[j][k]) >= big) {
                                big = std::abs(minv[j][k]);
                                irow = j;
                                icol = k;
                            }
                        } else if (ipiv[k] > 1) {
                            FatalError("Singular matrix cannot be inverted");
                        }
                    }
                }                
            }
            ++ipiv[icol];

            // Swap pivot row
            if (irow != icol) {
                for (int k = 0; k < 4; k++) {
                    std::swap(minv[irow][k], minv[icol][k]);
                }
            }

            indxr[i] = irow;
            indxc[i] = icol;
            if (std::abs(minv[icol][icol]) < EPS) {
                FatalError("Singular matrix cannot be inverted");
            }

            const double pinv = 1.0 / minv[icol][icol];
            minv[icol][icol] = 1.0;
            for (int j = 0; j < 4; j++) {
                minv[icol][j] *= pinv;
            }

            // Subtract diagonal value from the other rows
            for (int j = 0; j < 4; j++) {
                if (j != icol) {
                    double save = minv[j][icol];
                    minv[j][icol] = 0.0;
                    for (int k = 0; k < 4; k++) {
                        minv[j][k] -= minv[icol][k] * save;
                    }
                }
            }
        }

        // Swap columins to reset permutations
        for (int j = 3; j >= 0; j--) {
            if (indxr[j] != indxc[j]) {
                for (int k = 0; k < 4; k++) {
                    std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
                }
            }
        }

        return Matrix4x4(minv);
    }

    Matrix4x4& Matrix4x4::operator=(const Matrix4x4& mat) {
        if (this == &mat) return *this;

        memcpy(m, mat.m, sizeof(m));
        return *this;
    }

    bool Matrix4x4::operator==(const Matrix4x4& m2) const {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (m[i][j] != m2.m[i][j]) return false;
            }
        }
        return true;
    }

    bool Matrix4x4::operator!=(const Matrix4x4& m2) const {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (m[i][j] != m2.m[i][j]) return true;
            }
        }
        return false;
    }

    double Matrix4x4::operator()(int i, int j) const {
        Assertion(i >= 0 && i < 4 && j >= 0 && j < 4,
                  "Index out of bounds!!");
        return m[i][j];
    }

    std::string Matrix4x4::toString() const {
        std::stringstream ss;
        ss << std::fixed;
        ss << std::setprecision(8);
        ss << "[ ";
        for (int i = 0; i < 4; i++) {
            ss << "[ ";
            for (int j = 0; j < 4; j++) {
                ss << m[i][j];
                if (j != 3) {
                    ss << ", ";
                }
            }
            ss << "] ";

            if (i != 3) {
                ss << std::endl << "  ";
            }
        }
        ss << "]";
        return std::move(ss.str());
    }


}  // namespace spica

spica::Matrix4x4 operator*(const spica::Matrix4x4& m1, const spica::Matrix4x4& m2) {
    double ret[4][4];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ret[i][j] = 0.0;
            for (int k = 0; k < 4; k++) {
                ret[i][j] += m1(i, k) * m2(k, j);            
            }
        }
    }
    return spica::Matrix4x4(ret);
}

std::ostream& operator<<(std::ostream& os, const spica::Matrix4x4& mat) {
    os << mat.toString();
    return os;
}
