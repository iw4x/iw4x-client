# Code Style

```c++
namespace SomeNamespace
{
	class SomeClass
	{
	public:
		int someVariable;
		static int SomeStaticVariable;

		void someMethod()
		{
			// [...]
		}

		static void SomeStaticFunction()
		{
			// [...]
		}
	};
}

void Main(int argument)
{
	SomeNamespace::SomeClass someObject;
	someObject.someVariable = 0;
	someObject.someMethod();

	SomeNamespace::SomeClass::SomeStaticVariable = 0;
	SomeNamespace::SomeClass::SomeStaticFunction();
}
```