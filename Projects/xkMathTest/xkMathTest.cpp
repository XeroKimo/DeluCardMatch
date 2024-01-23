#include "CppUnitTest.h"
#include <sstream>
#include "Insanity_Math.h"


import xk.Math.Matrix;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace InsanityEngine::Math::Types;
using namespace InsanityEngine;

namespace xkma = xk::Math::Aliases;
namespace xkm = xk::Math;

template<class Ty, size_t M, size_t N>
void PrintMatrix(const xkm::Matrix<Ty, M, N>& mat)
{
    std::stringstream stream;

    for (size_t y = 0; y < N; y++)
    {
        for (size_t x = 0; x < M; x++)
            stream << mat.At(y, x) << " ";

        stream << "\n";
    }
    stream << "\n";

    Logger::WriteMessage(stream.str().c_str());

}

void PrintMatrix(const Matrix4x4f& mat)
{
    std::stringstream stream;

    for (size_t y = 0; y < 4; y++)
    {
        for (size_t x = 0; x < 4; x++)
            stream << mat(y, x) << " ";

        stream << "\n";
    }
    stream << "\n";

    Logger::WriteMessage(stream.str().c_str());

}


namespace INSANITYMATHTEST
{
    TEST_CLASS(INSANITYMATHTEST)
    {
    public:

        TEST_METHOD(VectorAddTest)
        {
            Vector3f one{ 1, 2, 3 };
            Vector3f two{ 4, 5, 6 };
            Vector3f three = one + two;

            xkma::Vector3 gOne{ 1, 2, 3 };
            xkma::Vector3 gTwo{ 4, 5, 6 };
            xkma::Vector3 gThree = gOne + gTwo;

            for (size_t i = 0; i < three.size; i++)
            {
                Assert::AreEqual(gThree[i], three[i]);
            }


        }



        TEST_METHOD(VectorSubtractTest)
        {
            Vector3f one{ 1, 2, 3 };
            Vector3f two{ 4, 5, 6 };
            Vector3f three = one - two;

            xkma::Vector3 gOne{ 1, 2, 3 };
            xkma::Vector3 gTwo{ 4, 5, 6 };
            xkma::Vector3 gThree = gOne - gTwo;

            for (size_t i = 0; i < three.size; i++)
            {
                Assert::AreEqual(gThree[i], three[i]);
            }
        }

        //TEST_METHOD(VectorMulTest)
        //{
        //    Vector3f one{ 1, 2, 3 };
        //    Vector3f two{ 4, 5, 6 };
        //    Vector3f three = one * two;

        //    xkma::Vector3 gOne{ 1, 2, 3 };
        //    xkma::Vector3 gTwo{ 4, 5, 6 };
        //    xkma::Vector3 gThree = gOne * gTwo;

        //    for (size_t i = 0; i < three.size; i++)
        //    {
        //        Assert::AreEqual(gThree[i], three[i]);
        //    }
        //}

        //TEST_METHOD(VectorDivTest)
        //{
        //    Vector3f one{ 1, 2, 3 };
        //    Vector3f two{ 4, 5, 6 };
        //    Vector3f three = one / two;

        //    xkma::Vector3 gOne{ 1, 2, 3 };
        //    xkma::Vector3 gTwo{ 4, 5, 6 };
        //    xkma::Vector3 gThree = gOne / gTwo;

        //    for (size_t i = 0; i < three.size; i++)
        //    {
        //        Assert::AreEqual(gThree[i], three[i]);
        //    }
        //}


        //TEST_METHOD(VectorFunctionsTest)
        //{
        //    Vector3f one{ 1, 2, 3 };
        //    Vector3f two{ 4, 5, 6 };
        //    xkma::Vector3 gOne{ 1, 2, 3 };
        //    xkma::Vector3 gTwo{ 4, 5, 6 };


        //    Assert::AreEqual(Math::Vector::Magnitude(one), glm::length(gOne));

        //    Assert::AreEqual(Math::Vector::Dot(one, two), glm::dot(gOne, gTwo));

        //    Vector3f three = Math::Vector::Cross(one, two);
        //    xkma::Vector3 gThree = glm::cross(gOne, gTwo);

        //    for (size_t i = 0; i < three.size; i++)
        //    {
        //        Assert::AreEqual(gThree[i], three[i]);
        //    }

        //}


        TEST_METHOD(MatrixAccessTest)
        {
            Matrix4x4f one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            xkm::Matrix<float, 4, 4> gOne
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };

            for (int y = 0; y < 4; y++)
            {
                for (int x = 0; x < 4; x++)
                {
                    Assert::AreEqual(one.At(y, x), gOne.At(y, x));
                }
            }

        }

        TEST_METHOD(MatrixEqualityTest)
        {
            xkm::Matrix<float, 4, 4> one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            xkm::Matrix<float, 4, 4> two = one;


            Assert::AreEqual(one == two, true);
            Assert::AreEqual(one != two, false);
        }


        TEST_METHOD(MatrixAddTest)
        {
            Matrix4x4f one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            Matrix4x4f two
            {
                17, 18, 19, 20,
                21, 22, 23, 24,
                25, 26, 27, 28,
                29, 30, 31, 32
            };
            Matrix4x4f three = one + two;

            xkm::Matrix<float, 4, 4> gOne
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            xkm::Matrix<float, 4, 4> gTwo
            {
                17, 18, 19, 20,
                21, 22, 23, 24,
                25, 26, 27, 28,
                29, 30, 31, 32
            };
            xkm::Matrix<float, 4, 4> gThree = gOne + gTwo;

            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(gThree.At(y, x), three(y, x));
            }
        }



        TEST_METHOD(MatrixSubtractTest)
        {
            Matrix4x4f one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            Matrix4x4f two
            {
                17, 18, 19, 20,
                21, 22, 23, 24,
                25, 26, 27, 28,
                29, 30, 31, 32
            };
            Matrix4x4f three = one - two;

            xkm::Matrix<float, 4, 4> gOne
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            xkm::Matrix<float, 4, 4> gTwo
            {
                17, 18, 19, 20,
                21, 22, 23, 24,
                25, 26, 27, 28,
                29, 30, 31, 32
            };
            xkm::Matrix<float, 4, 4> gThree = gOne - gTwo;

            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(gThree.At(y, x), three(y, x));
            }
        }

        TEST_METHOD(SquareMatrixElementRefTest)
        {
            xkm::Matrix<float, 4, 4> gOne
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };

            xkm::RowRef r1{ gOne, 0 };
            xkm::RowRef r2{ gOne, 1 };
            xkm::RowRef r3{ gOne, 2 };
            xkm::RowRef r4{ gOne, 3 };

            Assert::AreEqual(1.f, r1[0]);
            Assert::AreEqual(2.f, r1[1]);
            Assert::AreEqual(3.f, r1[2]);
            Assert::AreEqual(4.f, r1[3]);

            Assert::AreEqual(5.f, r2[0]);
            Assert::AreEqual(6.f, r2[1]);
            Assert::AreEqual(7.f, r2[2]);
            Assert::AreEqual(8.f, r2[3]);

            Assert::AreEqual(9.f, r3[0]);
            Assert::AreEqual(10.f, r3[1]);
            Assert::AreEqual(11.f, r3[2]);
            Assert::AreEqual(12.f, r3[3]);

            Assert::AreEqual(13.f, r4[0]);
            Assert::AreEqual(14.f, r4[1]);
            Assert::AreEqual(15.f, r4[2]);
            Assert::AreEqual(16.f, r4[3]);

            xkm::ColumnRef c1{ gOne, 0 };
            xkm::ColumnRef c2{ gOne, 1 };
            xkm::ColumnRef c3{ gOne, 2 };
            xkm::ColumnRef c4{ gOne, 3 };


            Assert::AreEqual(1.f, c1[0]);
            Assert::AreEqual(5.f, c1[1]);
            Assert::AreEqual(9.f, c1[2]);
            Assert::AreEqual(13.f, c1[3]);

            Assert::AreEqual(2.f, c2[0]);
            Assert::AreEqual(6.f, c2[1]);
            Assert::AreEqual(10.f, c2[2]);
            Assert::AreEqual(14.f, c2[3]);

            Assert::AreEqual(3.f,  c3[0]);
            Assert::AreEqual(7.f, c3[1]);
            Assert::AreEqual(11.f, c3[2]);
            Assert::AreEqual(15.f, c3[3]);

            Assert::AreEqual(4.f, c4[0]);
            Assert::AreEqual(8.f, c4[1]);
            Assert::AreEqual(12.f, c4[2]);
            Assert::AreEqual(16.f, c4[3]);
            
        }

        TEST_METHOD(NonSquareMatrixElementRefTest)
        {
            {
                xkm::Matrix<float, 2, 3> gOne
                {
                    1, 2, 3,
                    4, 5, 6
                };

                xkm::RowRef r1{ gOne, 0 };
                xkm::RowRef r2{ gOne, 1 };

                Assert::AreEqual(1.f, r1[0]);
                Assert::AreEqual(2.f, r1[1]);
                Assert::AreEqual(3.f, r1[2]);

                Assert::AreEqual(4.f, r2[0]);
                Assert::AreEqual(5.f, r2[1]);
                Assert::AreEqual(6.f, r2[2]);

                xkm::ColumnRef c1{ gOne, 0 };
                xkm::ColumnRef c2{ gOne, 1 };
                xkm::ColumnRef c3{ gOne, 2 };

                Assert::AreEqual(1.f, c1[0]);
                Assert::AreEqual(4.f, c1[1]);

                Assert::AreEqual(2.f, c2[0]);
                Assert::AreEqual(5.f, c2[1]);

                Assert::AreEqual(3.f, c3[0]);
                Assert::AreEqual(6.f, c3[1]);
            }
            {
                xkm::Matrix<float, 3, 2> gOne
                {
                    1, 2,
                    3, 4,
                    5, 6
                };


                xkm::RowRef r1{ gOne, 0 };
                xkm::RowRef r2{ gOne, 1 };
                xkm::RowRef r3{ gOne, 2 };

                Assert::AreEqual(1.f, r1[0]);
                Assert::AreEqual(2.f, r1[1]);

                Assert::AreEqual(3.f, r2[0]);
                Assert::AreEqual(4.f, r2[1]);

                Assert::AreEqual(5.f, r3[0]);
                Assert::AreEqual(6.f, r3[1]);

                xkm::ColumnRef c1{ gOne, 0 };
                xkm::ColumnRef c2{ gOne, 1 };

                Assert::AreEqual(1.f, c1[0]);
                Assert::AreEqual(3.f, c1[1]);
                Assert::AreEqual(5.f, c1[2]);

                Assert::AreEqual(2.f, c2[0]);
                Assert::AreEqual(4.f, c2[1]);
                Assert::AreEqual(6.f, c2[2]);
            }
        }

        TEST_METHOD(SquareMatrixMulTest)
        {
            Matrix4x4f one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            Matrix4x4f two
            {
                17, 18, 19, 20,
                21, 22, 23, 24,
                25, 26, 27, 28,
                29, 30, 31, 32
            };
            Matrix4x4f three = one * two;

            xkm::Matrix<float, 4, 4> gOne
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };
            xkm::Matrix<float, 4, 4> gTwo
            {
                17, 18, 19, 20,
                21, 22, 23, 24,
                25, 26, 27, 28,
                29, 30, 31, 32
            };
            xkm::Matrix<float, 4, 4> gThree = gOne * gTwo;

            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(gThree.At(y, x), three.At(y, x));
            }

        }

        TEST_METHOD(NonSquareMatrixMulTest)
        {

            Matrix<float, 2, 3> f1
            {
                1, 2, 3,
                4, 5, 6
            };
            Matrix<float, 3, 2> f2
            {
                1, 2,
                3, 4,
                5, 6
            };

            Matrix<float, 3, 3> f3 = f2 * f1;

            xkm::Matrix<float, 2, 3> gf1
            {
                1, 2, 3,
                4, 5, 6
            };

            xkm::Matrix<float, 3, 2> gf2
            {
                1, 2,
                3, 4,
                5, 6
            };

            xkm::Matrix<float, 3, 3>  gf3 = gf2 * gf1;

            for (size_t y = 0; y < Matrix<float, 3, 3> ::row_count; y++)
            {
                for (size_t x = 0; x < Matrix<float, 3, 3> ::column_count; x++)
                    Assert::AreEqual(gf3.At(y, x), f3.At(y, x));
            }
        }


        TEST_METHOD(MatrixVectorTest)
        {
            Matrix4x4f one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };

            Vector4f two{ 5, 6, 7, 8 };
            Vector4f three = one * two;

            xkm::Matrix<float, 4, 4> gOne
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };

            xkma::Vector4 gTwo{ 5, 6, 7, 8 };
            xkma::Vector4 gThree = gOne * gTwo;

            for (size_t i = 0; i < three.size; i++)
            {
                Assert::AreEqual(gThree[i], three[i]);
            }

        }

        TEST_METHOD(MatrixTranslationTest)
        {

            Matrix4x4f one
            {
                1, 0, 0, 1,
                0, 1, 0, 2,
                0, 0, 1, 3,
                0, 0, 0, 1
            };

            Matrix4x4f two
            {
                1, 0, 0, 9,
                0, 1, 0, 8,
                0, 0, 1, 7,
                0, 0, 0, 1
            };

            Matrix4x4f three = two * one;

            xkm::Matrix<float, 4, 4> gOne
            {
                1, 0, 0, 1,
                0, 1, 0, 2,
                0, 0, 1, 3,
                0, 0, 0, 1
            };

            xkm::Matrix<float, 4, 4> gTwo
            {
                1, 0, 0, 9,
                0, 1, 0, 8,
                0, 0, 1, 7,
                0, 0, 0, 1
            };


            xkm::Matrix<float, 4, 4> gThree = gTwo * gOne;



            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(gThree.At(y, x), three(y, x));
            }
        }


        TEST_METHOD(MatrixFunctionTest)
        {
            xkm::Matrix<float, 4, 4> gOne =
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };

            xkm::Matrix<float, 4, 4> gTwo = Transpose(gOne);

            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(gOne.At(y, x), gTwo.At(x, y));
            }

            xkm::Matrix<float, 4, 4> transform1 = xkm::TransformMatrix(xkma::Vector3{ 1, 2, 3 });
            auto transform2 = xkm::Matrix<float, 4, 4>::Identity();
            transform2.At(0, 3) = 1;
            transform2.At(1, 3) = 2;
            transform2.At(2, 3) = 3;

            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(transform1.At(y, x), transform2.At(y, x));
            }
        }


        TEST_METHOD(MatrixIdentityTest)
        {
            Matrix4x4f one
            {
                1, 2, 3, 4,
                5, 6, 7, 8,
                9, 10, 11, 12,
                13, 14, 15, 16
            };

            Matrix4x4f two = Matrix4x4f::Identity();
            Matrix4x4f three = one * Matrix4x4f::Identity();


            for (size_t y = 0; y < Matrix4x4f::row_count; y++)
            {
                for (size_t x = 0; x < Matrix4x4f::column_count; x++)
                    Assert::AreEqual(one(y, x), three(y, x));
            }
        }

        TEST_METHOD(TrigTest)
        {
            Degrees<float> d = Degrees(180.f);
            Radians<float> r = d.ToRadians();

            Assert::AreEqual(r.Data(), d.Data() / 180.f * Math::Constants::pi<float>);
        }


        TEST_METHOD(EulerQuaternion)
        {
            Quaternion<float> one{ Degrees<float>(90), Degrees<float>(0), Degrees<float>(0) };

            Assert::IsTrue(one.ToEulerDegrees().x() >= 89.9f || one.ToEulerDegrees().x() <= 90.1f);

            Quaternion<float> two{ Degrees<float>(0), Degrees<float>(90), Degrees<float>(0) };

            Assert::IsTrue(two.ToEulerDegrees().y() >= 89.9f || two.ToEulerDegrees().y() <= 90.1f);

            Quaternion<float> three{ Degrees<float>(0), Degrees<float>(0), Degrees<float>(90) };

            Assert::IsTrue(three.ToEulerDegrees().z() >= 89.9f || three.ToEulerDegrees().z() <= 90.1f);

            Quaternion<float> four = one * two;

            Assert::IsTrue(
                (four.ToEulerDegrees().y() >= 89.9f || four.ToEulerDegrees().y() <= 90.1f) &&
                (four.ToEulerDegrees().x() >= 89.9f || four.ToEulerDegrees().x() <= 90.1f));

            one = Quaternion(Degrees<float>(-90), Degrees<float>(0), Degrees<float>(0));

            Assert::IsTrue(one.ToEulerDegrees().x() <= -89.9f || one.ToEulerDegrees().x() >= -90.1f);

            two = Quaternion(Degrees<float>(0), Degrees<float>(-90), Degrees<float>(0));

            Assert::IsTrue(two.ToEulerDegrees().y() <= -89.9f || two.ToEulerDegrees().y() >= -90.1f);

            three = Quaternion(Degrees<float>(0), Degrees<float>(0), Degrees<float>(-90));

            Assert::IsTrue(three.ToEulerDegrees().z() <= -89.9f || three.ToEulerDegrees().z() >= -90.1f);

            four = one * two;

            Assert::IsTrue(
                (four.ToEulerDegrees().y() <= -89.9f || four.ToEulerDegrees().y() >= -90.1f) &&
                (four.ToEulerDegrees().x() <= -89.9f || four.ToEulerDegrees().x() >= -90.1f));

            four = two * one;

            Assert::IsTrue(
                (four.ToEulerDegrees().y() <= -89.9f || four.ToEulerDegrees().y() >= -90.1f) &&
                (four.ToEulerDegrees().x() <= -89.9f || four.ToEulerDegrees().x() >= -90.1f));
        }

        TEST_METHOD(AxisQuaternion)
        {
            Quaternion<float> one{ { 1, 0, 0 }, Degrees<float>(90) };
            Assert::IsTrue(one.ToEulerDegrees().x() >= 89.9f || one.ToEulerDegrees().x() <= 90.1f);

            Quaternion<float> two{ { 0, 1, 0 }, Degrees<float>(90) };
            Assert::IsTrue(two.ToEulerDegrees().y() >= 89.9f || two.ToEulerDegrees().y() <= 90.1f);

            Quaternion<float> three{ { 0, 0, 1 }, Degrees<float>(90) };
            Assert::IsTrue(three.ToEulerDegrees().z() >= 89.9f || three.ToEulerDegrees().z() <= 90.1f);

            one = Quaternion<float>{ { 1, 0, 0 }, Degrees<float>(45) };
            Assert::IsTrue(one.ToEulerDegrees().x() >= 44.9f || one.ToEulerDegrees().x() <= 46.1f);

            two = Quaternion<float>{ { 0, 1, 0 }, Degrees<float>(45) };
            Assert::IsTrue(two.ToEulerDegrees().y() >= 44.9f || two.ToEulerDegrees().y() <= 46.1f);

            three = Quaternion<float>{ { 0, 0, 1 }, Degrees<float>(45) };
            Assert::IsTrue(three.ToEulerDegrees().z() >= 44.9f || three.ToEulerDegrees().z() <= 46.1f);
        }

        TEST_METHOD(Quaternions)
        {
            Quaternion<float> one;

            one *= one;

            Assert::IsTrue(one == Quaternion<float>());

            Assert::IsTrue(Quaternion<float>(Degrees<float>(45.f), Degrees<float>(), Degrees<float>()) == Quaternion<float>({ 1, 0, 0 }, Degrees<float>(45.f)));
            Assert::IsTrue(Quaternion<float>(Degrees<float>(), Degrees<float>(45.f), Degrees<float>()) == Quaternion<float>({ 0, 1, 0 }, Degrees<float>(45.f)));
            Assert::IsTrue(Quaternion<float>(Degrees<float>(), Degrees<float>(), Degrees<float>(45.f)) == Quaternion<float>({ 0, 0, 1 }, Degrees<float>(45.f)));
        }

        TEST_METHOD(QuaternionEulerValues)
        {
            Quaternion<float> xPos;
            Quaternion<float> xNeg;

            Quaternion<float> yPos;
            Quaternion<float> yNeg;

            Quaternion<float> zPos;
            Quaternion<float> zNeg;

            constexpr float rotation = 30;

            xPos *= Quaternion<float>(Vector3f{ 1, 0, 0 }, Degreesf(rotation));
            xNeg *= Quaternion<float>(Vector3f{ 1, 0, 0 }, Degreesf(-rotation));

            yPos *= Quaternion<float>(Vector3f{ 0, 1, 0 }, Degreesf(rotation));
            yNeg *= Quaternion<float>(Vector3f{ 0, 1, 0 }, Degreesf(-rotation));

            zPos *= Quaternion<float>(Vector3f{ 0, 0, 1 }, Degreesf(rotation));
            zNeg *= Quaternion<float>(Vector3f{ 0, 0, 1 }, Degreesf(-rotation));



            std::stringstream stream;

            auto vec = xPos.ToEulerDegrees();
            stream << vec.x() << ", " << vec.y() << ", " << vec.z() << "\n";
            vec = xNeg.ToEulerDegrees();
            stream << vec.x() << ", " << vec.y() << ", " << vec.z() << "\n";

            vec = yPos.ToEulerDegrees();
            stream << vec.x() << ", " << vec.y() << ", " << vec.z() << "\n";
            vec = yNeg.ToEulerDegrees();
            stream << vec.x() << ", " << vec.y() << ", " << vec.z() << "\n";

            vec = zPos.ToEulerDegrees();
            stream << vec.x() << ", " << vec.y() << ", " << vec.z() << "\n";
            vec = zNeg.ToEulerDegrees();
            stream << vec.x() << ", " << vec.y() << ", " << vec.z() << "\n";

            Logger::WriteMessage(stream.str().c_str());
        }
    };
}
