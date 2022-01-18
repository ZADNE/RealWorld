#pragma once
#include <RealWorld/furniture/FX/FStatic.hpp>
#include <RealWorld/world/DestroyTicket.hpp>

class FLightS: public FStatic{
public:
	FLightS(const FStatic& f) : FStatic(f) {

	};

	void build() override;

	void destroy() override;
protected:
	DestroyTicket<StaticLight> p_sl;
};