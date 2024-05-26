//#include "CppUnitTest.h"
//
//using namespace Microsoft::VisualStudio::CppUnitTestFramework;
//import ECS;
//import xk.Math.Matrix;
//using namespace ECS;
//using namespace xk::Math;
//namespace ECSLIBTEST
//{
//	TEST_CLASS(ParentingTests)
//	{
//	public:
//		
//		TEST_METHOD(AddingParent)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			Assert::IsTrue(t2.GetParent() == &t1);
//			Assert::IsTrue(t1.GetChildren()[0] == &t2);
//		}
//		
//		TEST_METHOD(RemovingParent)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//			t2.SetParent(nullptr);
//
//			Assert::IsTrue(t2.GetParent() == nullptr);
//			Assert::IsTrue(t1.GetChildren().size() == 0);
//		}
//		
//		TEST_METHOD(RemovingParentViaDestructor)
//		{
//			TransformNode* t1 = new TransformNode();
//			TransformNode t2;
//
//			t2.SetParent(t1);
//			delete t1;
//
//			Assert::IsTrue(t2.GetParent() == nullptr);
//		}
//		
//		TEST_METHOD(RemovingChildViaDestructor)
//		{
//			TransformNode t1;
//			{
//				TransformNode t2;
//				t2.SetParent(&t1);
//			}
//
//			Assert::IsTrue(t1.GetChildren().size() == 0);
//		}
//		
//		TEST_METHOD(TwoStacksDeep)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//			t3.SetParent(&t2);
//
//			Assert::IsTrue(t2.GetParent() == &t1);
//			Assert::IsTrue(t1.GetChildren()[0] == &t2);
//
//			Assert::IsTrue(t3.GetParent() == &t2);
//			Assert::IsTrue(t2.GetChildren()[0] == &t3);
//		}
//		
//		TEST_METHOD(ReparentingTest)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//			t3.SetParent(&t2);
//
//			Assert::IsTrue(t2.GetParent() == &t1);
//			Assert::IsTrue(t1.GetChildren()[0] == &t2);
//
//			Assert::IsTrue(t3.GetParent() == &t2);
//			Assert::IsTrue(t2.GetChildren()[0] == &t3);
//
//			t3.SetParent(&t1);
//
//			Assert::IsTrue(t3.GetParent() == &t1);
//			Assert::IsTrue(t1.GetChildren()[1] == &t3);
//		}
//		
//		TEST_METHOD(ReparentingViaDestructor)
//		{
//			TransformNode t1;
//			TransformNode* t2 = new TransformNode();
//			TransformNode t3;
//
//			t2->SetParent(&t1);
//			t3.SetParent(t2);
//
//			delete t2;
//
//			Assert::IsTrue(t3.GetParent() == &t1);
//			Assert::IsTrue(t1.GetChildren()[0] == &t3);
//		}
//		
//		TEST_METHOD(CyclicDetectionTest)
//		{
//			Assert::ExpectException<std::exception>([t1 = TransformNode{}, t2 = TransformNode{}]() mutable
//				{
//					t1.SetParent(&t2);
//					t2.SetParent(&t1);
//				});
//		}
//		
//		TEST_METHOD(ComplexCyclicDetectionTest)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//			t3.SetParent(&t2);
//
//			Assert::ExpectException<std::exception>([&]() mutable
//				{
//					t2.SetParent(&t3);
//				});
//
//			Assert::ExpectException<std::exception>([&]() mutable
//				{
//					t1.SetParent(&t3);
//				});
//		}
//		
//		TEST_METHOD(SelfParentingTest)
//		{
//			Assert::ExpectException<std::exception>([t1 = TransformNode{}]() mutable
//				{
//					t1.SetParent(&t1);
//				});
//		}
//	};
//
//	TEST_CLASS(WorldTransformationPositionTests)
//	{
//	public:
//		
//		TEST_METHOD(ParentPositionChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = { 10.f, 10.f };
//
//			Assert::IsTrue(w1.Position().GetValue() == w2.Position().GetValue());
//			Assert::IsTrue(t1.GetLocalPosition() != t2.GetLocalPosition());
//			Assert::IsTrue(t1.GetLocalPosition() == Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(t2.GetLocalPosition() == Vector<float, 2>{});
//		}
//		
//		TEST_METHOD(ChildPositionChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w2.Position() = { 10.f, 10.f };
//			Assert::IsTrue(w1.Position().GetValue() == xk::Math::Vector<float, 2>{0, 0});
//			Assert::IsTrue(w2.Position().GetValue() == xk::Math::Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(t1.GetLocalPosition() == xk::Math::Vector<float, 2>{});
//			Assert::IsTrue(t2.GetLocalPosition() == xk::Math::Vector<float, 2>{ 10.f, 10.f });
//		}
//		
//		TEST_METHOD(ParentAndChildSamePositionChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = { 10.f, 10.f };
//			w2.Position() = { 10.f, 10.f };
//
//			Assert::IsTrue(w1.Position().GetValue() == w2.Position().GetValue());
//			Assert::IsTrue(t1.GetLocalPosition() == Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(t2.GetLocalPosition() == xk::Math::Vector<float, 2>{ 0.f, 0.f });
//		}
//		
//		TEST_METHOD(ParentAndChildDifferentPositionChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = { 10.f, 10.f };
//			w2.Position() = { 0.f, 0.f };
//			Assert::IsTrue(w1.Position().GetValue() == w2.Position().GetValue() - t2.GetLocalPosition());
//			Assert::IsTrue(w2.Position().GetValue() == Vector<float, 2>{ 0.f, 0.f });
//			Assert::IsTrue(t1.GetLocalPosition() == Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(t2.GetLocalPosition() == xk::Math::Vector<float, 2>{ -10.f, -10.f });
//		}
//		
//		TEST_METHOD(UnparentChangePositionKeepWorldTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = { 10.f, 10.f };
//
//			t2.SetParent(nullptr);
//
//			Assert::IsTrue(w1.Position().GetValue() == w2.Position().GetValue());
//			Assert::IsTrue(t1.GetLocalPosition() == xk::Math::Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(t2.GetLocalPosition() == xk::Math::Vector<float, 2>{ 10.f, 10.f });
//		}
//		
//		TEST_METHOD(UnparentChangePositionKeepLocalTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = { 10.f, 10.f };
//
//			t2.SetParent(nullptr, ReparentLogic::KeepLocalTransform);
//
//			Assert::IsTrue(w1.Position().GetValue() != w2.Position().GetValue());
//			Assert::IsTrue(w2.Position().GetValue() == t2.GetLocalPosition());
//			Assert::IsTrue(t1.GetLocalPosition() == xk::Math::Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(t2.GetLocalPosition() == xk::Math::Vector<float, 2>{ 0.f, 0.f });
//		}
//		
//		TEST_METHOD(ReparentChangePositionKeepWorldTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//			WorldTransformProxy w3{ t3 };
//
//			w1.Position() = { 10.f, 10.f };
//			w3.Position() = { -40, 100 };
//
//			Assert::IsTrue(w1.Position().GetValue() == w2.Position().GetValue());
//			t2.SetParent(&t3);
//
//
//			Assert::IsTrue(w2.Position().GetValue() != w3.Position().GetValue());
//			Assert::IsTrue(w1.Position().GetValue() == Vector<float, 2>{ 10.f, 10.f});
//			Assert::IsTrue(w3.Position().GetValue() == Vector<float, 2>{ -40, 100 });
//			Assert::IsTrue(w2.Position().GetValue() == w1.Position().GetValue());
//			Assert::IsTrue(t2.GetLocalPosition() == Vector<float, 2>{ 50, -90 });
//		}
//		
//		TEST_METHOD(ReparentChangePositionKeepLocalTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//			WorldTransformProxy w3{ t3 };
//
//			w1.Position() = { 10.f, 10.f };
//			w3.Position() = { -40, 100 };
//
//			Assert::IsTrue(w1.Position().GetValue() == w2.Position().GetValue());
//			t2.SetParent(&t3, ReparentLogic::KeepLocalTransform);
//
//			Assert::IsTrue(w2.Position().GetValue() == w3.Position().GetValue());
//
//			Assert::IsTrue(w1.Position().GetValue() == Vector<float, 2>{ 10.f, 10.f});
//			Assert::IsTrue(w3.Position().GetValue() == Vector<float, 2>{ -40, 100 });
//		}
//
//		TEST_METHOD(AccumulationTest)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = Vector<float, 2>{ 10, 10 };
//			w2.Position() += Vector<float, 2>{ 20, 20 };
//
//			Assert::IsTrue(w1.Position().GetValue() == Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(w2.Position().GetValue() == Vector<float, 2>{ 30.f, 30.f });
//		}
//
//		TEST_METHOD(SubtractionTest)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Position() = Vector<float, 2>{ 10, 10 };
//			w2.Position() -= Vector<float, 2>{ 20, 20 };
//
//			Assert::IsTrue(w1.Position().GetValue() == Vector<float, 2>{ 10.f, 10.f });
//			Assert::IsTrue(w2.Position().GetValue() == Vector<float, 2>{ -10.f, -10.f });
//		}
//	};
//
//	TEST_CLASS(WorldTransformationRotationTests)
//	{
//	public:
//		
//		TEST_METHOD(ParentRotationChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = { 10.f };
//
//			Assert::IsTrue(w1.Rotation().GetValue() == w2.Rotation().GetValue());
//			Assert::IsTrue(t1.GetLocalRotation() != t2.GetLocalRotation());
//			Assert::IsTrue(t1.GetLocalRotation() == Degree<float>{ 10.f });
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{});
//		}
//		
//		TEST_METHOD(ChildRotationChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w2.Rotation() = { 10.f };
//			Assert::IsTrue(w1.Rotation().GetValue() == Degree<float>{ 0 });
//			Assert::IsTrue(w2.Rotation().GetValue() == Degree<float>{ 10.f });
//			Assert::IsTrue(t1.GetLocalRotation() == Degree<float>{ });
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{ 10.f });
//		}
//		
//		TEST_METHOD(ParentAndChildSameRotationChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = { 10.f };
//			w2.Rotation() = { 10.f };
//
//			Assert::IsTrue(w1.Rotation().GetValue() == w2.Rotation().GetValue());
//			Assert::IsTrue(t1.GetLocalRotation() == Degree<float>{ 10.f });
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{ 0.f });
//		}
//		
//		TEST_METHOD(ParentAndChildDifferentRotationChange)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = { 10.f };
//			w2.Rotation() = { 0.f };
//			Assert::IsTrue(w1.Rotation().GetValue() == w2.Rotation().GetValue() - t2.GetLocalRotation());
//			Assert::IsTrue(w2.Rotation().GetValue() == Degree<float>{ 0.f });
//			Assert::IsTrue(t1.GetLocalRotation() == Degree<float>{ 10.f });
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{ -10.f });
//		}
//		
//		TEST_METHOD(UnparentChangeRotationKeepWorldTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = { 10.f };
//
//			t2.SetParent(nullptr);
//
//			Assert::IsTrue(w1.Rotation().GetValue() == w2.Rotation().GetValue());
//			Assert::IsTrue(t1.GetLocalRotation() == Degree<float>{ 10.f });
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{ 10.f });
//		}
//		
//		TEST_METHOD(UnparentChangeRotationKeepLocalTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = { 10.f };
//
//			t2.SetParent(nullptr, ReparentLogic::KeepLocalTransform);
//
//			Assert::IsTrue(w1.Rotation().GetValue() != w2.Rotation().GetValue());
//			Assert::IsTrue(w2.Rotation().GetValue() == t2.GetLocalRotation());
//			Assert::IsTrue(t1.GetLocalRotation() == Degree<float>{ 10.f });
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{ 0.f });
//		}
//		
//		TEST_METHOD(ReparentChangeRotationKeepWorldTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//			WorldTransformProxy w3{ t3 };
//
//			w1.Rotation() = { 10.f };
//			w3.Rotation() = { -40 };
//
//			Assert::IsTrue(w1.Rotation().GetValue() == w2.Rotation().GetValue());
//			t2.SetParent(&t3);
//
//
//			Assert::IsTrue(w2.Rotation().GetValue() != w3.Rotation().GetValue());
//			Assert::IsTrue(w1.Rotation().GetValue() == Degree<float>{ 10.f });
//			Assert::IsTrue(w3.Rotation().GetValue() == Degree<float>{ -40 });
//			Assert::IsTrue(w2.Rotation().GetValue() == w1.Rotation().GetValue());
//			Assert::IsTrue(t2.GetLocalRotation() == Degree<float>{ 50 });
//		}
//		
//		TEST_METHOD(ReparentChangeRotationKeepLocalTransform)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//			WorldTransformProxy w3{ t3 };
//
//			w1.Rotation() = { 10.f };
//			w3.Rotation() = { -40 };
//
//			Assert::IsTrue(w1.Rotation().GetValue() == w2.Rotation().GetValue());
//			t2.SetParent(&t3, ReparentLogic::KeepLocalTransform);
//
//			Assert::IsTrue(w2.Rotation().GetValue() == w3.Rotation().GetValue());
//
//			Assert::IsTrue(w1.Rotation().GetValue() == Degree<float>{ 10.f });
//			Assert::IsTrue(w3.Rotation().GetValue() == Degree<float>{ -40 });
//		}
//
//		TEST_METHOD(AccumulationTest)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = Degree<float>{ 10 };
//			w2.Rotation() += Degree<float>{ 20};
//
//			Assert::IsTrue(w1.Rotation().GetValue() == Degree<float>{ 10.f });
//			Assert::IsTrue(w2.Rotation().GetValue() == Degree<float>{ 30.f });
//		}
//
//		TEST_METHOD(SubtractionTest)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			t2.SetParent(&t1);
//
//			WorldTransformProxy w1{ t1 };
//			WorldTransformProxy w2{ t2 };
//
//			w1.Rotation() = Degree<float>{ 10 };
//			w2.Rotation() -= Degree<float>{ 20};
//
//			Assert::IsTrue(w1.Rotation().GetValue() == Degree<float>{ 10.f });
//			Assert::IsTrue(w2.Rotation().GetValue() == Degree<float>{ -10.f });
//		}
//	};
//
//	TEST_CLASS(LocalTransformationTests)
//	{
//		TEST_METHOD(ProxyEquivalence)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			LocalTransformProxy l1(t1);
//			LocalTransformProxy l2(t2);
//
//			t2.SetParent(&t1);
//
//			l1.Position() = Vector<float, 2>{ 10, 10 };
//			l1.Rotation() = Degree<float>{ 10 };
//
//			l2.Position() = Vector<float, 2>{ 10, 10 };
//			l2.Rotation() = Degree<float>{ 10 };
//
//			Assert::IsTrue(l1.Position() == t1.GetLocalPosition());
//			Assert::IsTrue(l1.Rotation() == t1.GetLocalRotation());
//
//			Assert::IsTrue(l2.Position() == t2.GetLocalPosition());
//			Assert::IsTrue(l2.Rotation() == t2.GetLocalRotation());
//		}
//
//		TEST_METHOD(OneStackDeepLocation)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			LocalTransformProxy l1(t1);
//			LocalTransformProxy l2(t2);
//
//			WorldTransformProxy w1(t1);
//			WorldTransformProxy w2(t2);
//
//			t2.SetParent(&t1);
//
//			l1.Position() = Vector<float, 2>{ 10, 10 };
//			l2.Position() = Vector<float, 2>{ 10, 10 };
//
//			Assert::IsTrue(w1.Position() == Vector<float, 2>{ 10, 10 });
//			Assert::IsTrue(w2.Position() == Vector<float, 2>{ 20, 20 });
//			Assert::IsTrue(w2.Position() == l1.Position().GetValue() + l2.Position().GetValue());
//			Assert::IsTrue(w2.Position() == w1.Position().GetValue() + l2.Position().GetValue());
//		}
//
//		TEST_METHOD(OneStackDeepRotation)
//		{
//			TransformNode t1;
//			TransformNode t2;
//
//			LocalTransformProxy l1(t1);
//			LocalTransformProxy l2(t2);
//
//			WorldTransformProxy w1(t1);
//			WorldTransformProxy w2(t2);
//
//			t2.SetParent(&t1);
//
//			l1.Rotation() = Degree<float>{ 10 };
//			l2.Rotation() = Degree<float>{ 10 };
//
//			Assert::IsTrue(w1.Rotation() == Degree<float>{ 10 });
//			Assert::IsTrue(w2.Rotation() == Degree<float>{ 20 });
//			Assert::IsTrue(w2.Rotation() == l1.Rotation().GetValue() + l2.Rotation().GetValue());
//			Assert::IsTrue(w2.Rotation() == w1.Rotation().GetValue() + l2.Rotation().GetValue());
//		}
//
//		TEST_METHOD(TwoStackDeepLocation)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			LocalTransformProxy l1(t1);
//			LocalTransformProxy l2(t2);
//			LocalTransformProxy l3(t3);
//
//			WorldTransformProxy w1(t1);
//			WorldTransformProxy w2(t2);
//			WorldTransformProxy w3(t3);
//
//			t2.SetParent(&t1);
//			t3.SetParent(&t2);
//
//			l1.Position() = Vector<float, 2>{ 10, 10 };
//			l2.Position() = Vector<float, 2>{ 10, 10 };
//			l3.Position() = Vector<float, 2>{ 10, 10 };
//
//			Assert::IsTrue(w1.Position() == Vector<float, 2>{ 10, 10 });
//			Assert::IsTrue(w2.Position() == Vector<float, 2>{ 20, 20 });
//			Assert::IsTrue(w3.Position() == Vector<float, 2>{ 30, 30 });
//			Assert::IsTrue(w2.Position() == l1.Position().GetValue() + l2.Position().GetValue());
//			Assert::IsTrue(w2.Position() == w1.Position().GetValue() + l2.Position().GetValue());
//			Assert::IsTrue(w3.Position() == l1.Position().GetValue() + l2.Position().GetValue() + l3.Position().GetValue());
//			Assert::IsTrue(w3.Position() == w2.Position().GetValue() + l3.Position().GetValue());
//		}
//
//		TEST_METHOD(TwoStackDeepRotation)
//		{
//			TransformNode t1;
//			TransformNode t2;
//			TransformNode t3;
//
//			LocalTransformProxy l1(t1);
//			LocalTransformProxy l2(t2);
//			LocalTransformProxy l3(t3);
//
//			WorldTransformProxy w1(t1);
//			WorldTransformProxy w2(t2);
//			WorldTransformProxy w3(t3);
//
//			t2.SetParent(&t1);
//			t3.SetParent(&t2);
//
//			l1.Rotation() = Degree<float>{ 10 };
//			l2.Rotation() = Degree<float>{ 10 };
//			l3.Rotation() = Degree<float>{ 10 };
//
//			Assert::IsTrue(w1.Rotation() == Degree<float>{ 10 });
//			Assert::IsTrue(w2.Rotation() == Degree<float>{ 20 });
//			Assert::IsTrue(w3.Rotation() == Degree<float>{ 30 });
//			Assert::IsTrue(w2.Rotation() == l1.Rotation().GetValue() + l2.Rotation().GetValue());
//			Assert::IsTrue(w2.Rotation() == w1.Rotation().GetValue() + l2.Rotation().GetValue());
//			Assert::IsTrue(w3.Rotation() == l1.Rotation().GetValue() + l2.Rotation().GetValue() + l3.Rotation().GetValue());
//			Assert::IsTrue(w3.Rotation() == w2.Rotation().GetValue() + l3.Rotation().GetValue());
//		}
//	};
//}
