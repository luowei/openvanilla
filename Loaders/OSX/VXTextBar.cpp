// VXTextBar.cpp

#define OVDEBUG


#include "VXTextBar.h"
#include "VXUtility.h"

void VXTBSetRect(Rect *r, int fontsize, int textlen);
void VXTBSetRect(Rect *r, int fontsize, CFStringRef inString);

void VXTBFixPosition(Point *p, int width, int height);

VXTextBar::VXTextBar(int fsize) : fontsize(fsize)
{
    lookupdated=textupdated=posupdated=displaying=locked=0;
    pos.h=pos.v=20;
    text=CFStringCreateMutable(NULL, 0);
    CFStringAppendCString(text, " ", kCFStringEncodingMacRoman);
    
    // create window rectanble
    Rect windowrect, labelrect;
    SetRect(&labelrect, 5, 0, 24, 10);
    SetRect(&windowrect, 100, 100, 200, 40);
    // windowrect=labelrect;
        
  if (CreateNewWindow(kHelpWindowClass, 
        kWindowStandardHandlerAttribute, &windowrect, &window) != noErr) 
    {
        return;     // should throw exception
    }

    ControlFontStyleRec fontstyle;
    fontstyle.flags = kControlUseSizeMask | kControlUseBackColorMask | kControlUseForeColorMask; 
    
    fontstyle.size = fontsize;
    // It's the way to set background color; however, it seems it doesn't work.
    RGBColor color;
    color.red = 0xFF * 256;
    color.green = 0xFF * 256;
    color.blue = 0xFF * 256;
    fontstyle.backColor=color;
    color.red = 0x00 * 256;
    color.green = 0x00 * 256;
    color.blue = 0x00 * 256;
    fontstyle.foreColor=color; 

    CreateStaticTextControl(window, &labelrect, text, &fontstyle, &label);
    SetWindowAlpha(window, 0.7); //zonble
    MoveWindow(window, windowrect.left, windowrect.top, true);

}

VXTextBar::~VXTextBar()
{
    if (!window) return;
    HideWindow(window);
    DisposeWindow(window);
    CFRelease(text);
}

OVTextBar* VXTextBar::clear()
{
    CFRange rng=CFRangeMake(0,CFStringGetLength(text));
    CFStringDelete(text, rng);
    textupdated=1;
    return this;
}

OVTextBar* VXTextBar::append(void*s, OVEncoding e, int l)
{
    CFStringRef newstr=VXCreateCFString(s, e, l);
    CFStringAppend(text, newstr);
    CFRelease(newstr);
    textupdated=1;
    return this;
}

OVTextBar* VXTextBar::hide()
{
	if (locked)
	{
		clear();
		append((char*)"(選字窗)");
		update();
		return this;
	}
	
    if (displaying)
	{
		if (window) HideWindow(window);
		displaying=0;
	}
	return this;
}

OVTextBar* VXTextBar::show()
{
    update();
    if (displaying) return this;
    if (window) ShowWindow(window);
    displaying=1;
	return this;
}

VXTextBar* VXTextBar::lock()
{
	locked=1;
	
	return this;
}

VXTextBar* VXTextBar::unlock()
{
	locked=0;
	return this;
}

int VXTextBar::setPosition(int x, int y)
{
	if (!locked)
	{
		pos.h=x;
		pos.v=y;
	}
	else
	{
		Rect wbr;
		GetWindowBounds(window, kWindowGlobalPortRgn, &wbr);
		pos.h=wbr.left;
		pos.v=wbr.top;
	}

    return posupdated=1;
}

void VXTextBar::setFontSize(int s, int o, int *f, int *b)
{
	if (s < 6)
		return;
	if (s==fontsize && o == opacity && forecolor[0] == f[0] && forecolor[1] == f[1] && forecolor[2] == f[2] && backcolor[0] == b[0] && backcolor[1] == b[1] && backcolor[2] == b[2]) return;
	fontsize=s;
	lookupdated=1;
	opacity = o;
	forecolor[0] = f[0];
	forecolor[1] = f[1];
	forecolor[2] = f[2];
	backcolor[0] = b[0];
	backcolor[1] = b[1];
	backcolor[2] = b[2];
	
	ControlFontStyleRec fontstyle;
	fontstyle.flags = kControlUseSizeMask | kControlUseBackColorMask | kControlUseForeColorMask; 
	RGBColor color;
	color.red = backcolor[0];
	color.green = backcolor[1];
	color.blue = backcolor[2];
	murmur("back RGB: %d, %d, %d", backcolor[0], backcolor[1], backcolor[2]);
	fontstyle.backColor=color;

	color.red = forecolor[0];
	color.green = forecolor[1];
	color.blue = forecolor[2];
	murmur("fore RGB: %d, %d, %d", forecolor[0], forecolor[1], forecolor[2]);
	fontstyle.foreColor=color; 

	fontstyle.size=s;
	SetControlData(label, kControlEntireControl, 
			kControlFontStyleTag, sizeof(ControlFontStyleRec), 
			&fontstyle);
	SetWindowAlpha(window, (float)(opacity/100.0)); //zonble
	//SetWindowAlpha(window, 0.7); //zonble
	murmur("opacity: %f", (float)(opacity/100));
}

void VXTextBar::getPosition(int *x, int *y)
{
	*x=pos.h;
	*y=pos.v;
}

OVTextBar* VXTextBar::update()
{
    if (!(textupdated || lookupdated || posupdated)) return this;
    
    Rect windowrect, labelrect;

    SetPortWindowPort(window);
   
    if (textupdated)
    {
        if (SetControlData(label, kControlEntireControl, 
            kControlStaticTextCFStringTag, sizeof(CFStringRef), &text) != noErr)
                return this;   // should throw exception
        
        VXTBSetRect(&labelrect, fontsize, text);
        windowrect=labelrect;        
        SetControlBounds(label, &labelrect);
		
        if (!locked) 
		{
			SizeWindow(window, windowrect.right, windowrect.bottom, TRUE);
		}
		else
		{
			Rect bound=(*GetMainDevice())->gdRect;
			int w=bound.right-bound.left-25;
			labelrect.right=labelrect.left+w;
			SetControlBounds(label, &labelrect);
				
			SizeWindow(window, w, windowrect.bottom, TRUE);
		}

        murmur("textupdate: windowsize: right = %d , bottom = %d",windowrect.right, windowrect.bottom);

        textupdated=0;    
    }
    
    if (lookupdated)
    {
        VXTBSetRect(&labelrect, fontsize, text);
        windowrect=labelrect;        
        SetControlBounds(label, &labelrect);
        SizeWindow(window, windowrect.right, windowrect.bottom, TRUE);

        murmur("lookupdated: windowsize: right = %d , bottom = %d",windowrect.right, windowrect.bottom);

        lookupdated=0;
    }
    
    
    if (posupdated)
    {
        posupdated=0;
    }

    VXTBSetRect(&labelrect, fontsize, text);	
    VXTBFixPosition(&pos, labelrect.right, labelrect.bottom);
    MoveWindow(window, pos.h, pos.v, TRUE);
		
    DrawControls(window);
    return this;
}

void VXTBSetRect(Rect *r, int fontsize, int textlen)
{
    int w=textlen*fontsize;
    int h=fontsize;
    SetRect(r, 0, 0, w, h);
}

void VXTBFixPosition(Point *p, int width, int height)
{
	int heightfixer=height+20;
	int widthfixer=20;
	
	Rect bound, avail;
	GDHandle nextgd=GetDeviceList();
	
	do
	{
		if(!TestDeviceAttribute(nextgd, screenDevice)) continue;
		if(!TestDeviceAttribute(nextgd, screenActive)) continue;
		bound=(*nextgd)->gdRect;
		
		GetAvailableWindowPositioningBounds(nextgd, &avail);
		
		if (PtInRect(*p, &bound)) break;
	} while((nextgd = GetNextDevice(nextgd)) != nil);
	
	if (!PtInRect(*p, &bound))
	{
		bound=(*GetMainDevice())->gdRect;
		GetAvailableWindowPositioningBounds(nextgd, &avail);
	}
	
	if (p->v > avail.bottom - heightfixer) p->v = avail.bottom-heightfixer;
	if (p->h > avail.right - width) p->h = avail.right-width-widthfixer;
	if (p->v < avail.top+GetMBarHeight()) p->v=avail.top+GetMBarHeight();
	if (p->h < avail.left) p->h=avail.left+widthfixer;
}

#define IntToFixed(a)	   ((Fixed)(a) << 16)

void VXTBSetRect(Rect *r, int fontsize, CFStringRef inString)
{    
  ATSUStyle      Style;
  ATSUTextLayout Layout;
//  Fixed          fontSize = IntToFixed(fontsize * 8 / 5);

  Fixed          fontSize = IntToFixed(fontsize);

#define ATTRS 1
  ATSUAttributeTag      Tags[ATTRS];
  ATSUAttributeValuePtr Values[ATTRS];
  ByteCount             Sizes[ATTRS];
#undef ATTRS

  murmur("VXTBSetRect: Initializing.");

  Tags[0]   = kATSUSizeTag;
  Sizes[0]  = (ByteCount) sizeof(Fixed);
  Values[0] = &fontSize;

  ATSUCreateStyle(&Style);
  ATSUSetAttributes(Style,1,Tags,Sizes,Values);

  UniChar *Text, *RawText;
  UniCharCount RawLength = CFStringGetLength(inString);
  UniCharCount TextLength = 0;
  UniCharCount NonRomanCount=0;
  
  murmur("VXTBSetRect: Create text layout. TextLength = %d",(int)RawLength);
  RawText = (UniChar*) malloc (RawLength * sizeof(UniChar));
  Text = (UniChar*) malloc (RawLength * sizeof(UniChar));
  CFStringGetCharacters(inString,CFRangeMake(0,RawLength) ,RawText);

  for (UniCharCount i=0; i<RawLength; i++) {
    UniChar c=RawText[i];
    if (c < 0x80) {
        Text[TextLength++]=c;
    }
    else NonRomanCount++;
  }

  ATSUStyle Styles[1];        Styles[0]     = Style;
  UniCharCount runLengths[1]; runLengths[0] = TextLength;

  ATSUCreateTextLayoutWithTextPtr(Text,
		  0,
		  TextLength,
		  TextLength,
		  1,
		  runLengths,
		  Styles,
		  &Layout);

  murmur("VXTBSetRect: measure the Size.");

  Rect r2;
  ATSUMeasureTextImage(Layout,0,TextLength,
	kATSUUseGrafPortPenLoc,kATSUUseGrafPortPenLoc,&r2);

  murmur("VXTBSetRect: Measured: top = %d, left = %d, bottom = %d, right = %d",r2.top,r2.left, r2.bottom, r2.right);

//  SetRect(r, 0,0, r2.right - r2.left, r2.bottom - r2.top );

  murmur("RawLength=%d, TextLength=%d, NonRomanCount=%d",
    RawLength, TextLength, NonRomanCount);
  SetRect(r, 0,0, r2.right-r2.left+NonRomanCount*fontsize+7, fontsize+5);


  murmur("VXTBSetRect: done, dispose all objects");

  free(RawText);
  free(Text);
  ATSUDisposeStyle(Style);
  ATSUDisposeTextLayout(Layout);
}



