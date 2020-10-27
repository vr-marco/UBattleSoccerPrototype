/** MenuBackgroundWidgetStyle.h - This is the base class for background style widget blueprints accessed in the Unreal Editor.
*
* For a basic tutorial on creating menus with Slate, visit http://minalien.com/unreal-engine-4-creating-menus-with-slatec-part-1/
*
*/

#pragma once

#include "Styling/SlateWidgetStyleContainerBase.h"
#include "MenuBackgroundWidgetStyle.generated.h"

/**
* Represents the appearance of an FMenuItem
*/
USTRUCT()
struct FMenuBackgroundStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FMenuBackgroundStyle();
	virtual ~FMenuBackgroundStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FMenuBackgroundStyle& GetDefault();

	/**
	* The brush used for the menu background
	*/
	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateBrush BackgroundBrush;
	FMenuBackgroundStyle& SetBackgroundBrush(const FSlateBrush& InBackgroundBrush) { BackgroundBrush = InBackgroundBrush; return *this; }
};


/**
*/
UCLASS(hidecategories = Object, MinimalAPI)
class UMenuBackgroundWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	/** The actual data describing the menu's appearance. */
	UPROPERTY(Category = Appearance, EditAnywhere, meta = (ShowOnlyInnerProperties))
	FMenuBackgroundStyle BackgroundStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >(&BackgroundStyle);
	}
};
