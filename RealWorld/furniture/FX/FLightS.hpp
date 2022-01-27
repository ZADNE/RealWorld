#pragma once
#include <RealWorld/furniture/FX/FStatic.hpp>

class FLightS : public FStatic {
public:
	FLightS(const FStatic& f) : FStatic(f) {

	};

	void build() override;

	void destroy() override;
protected:

};