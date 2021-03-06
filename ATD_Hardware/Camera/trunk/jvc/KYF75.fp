s��        ��   � x  8   �   ����                               KYF75                           KYF75Ctrl 1.0 Type Library                                                                               � ��const char *     � ��LCID     	� 	��ERRORINFO  �  � ��HRESULT  � � ��SAFEARRAY *  � 	� 	��LPUNKNOWN     � ��VARIANT  � � ��VBOOL  �  � ��SCODE  � � ��CAObjHandle  � � ��DATE     � ��CURRENCY       KYF75Ctrl 1.0 Type Library         IKYF75 Interface    b    Use this function to create a new IKYF75 object, and obtain a handle to the object.

If the server application is already running, this function may or may not start another copy of the application.  This is determined by the server application.

You must call CA_InitActiveXThreadStyleForCurrentThread with COINIT_APARTMENTTHREADED if you register any ActiveX event callbacks and want the callbacks to be called from the same thread in which they were registered.  If you do not call CA_InitActiveXThreadStyleForCurrentThread with COINIT_APARTMENTTHREADED your callbacks will be called from a system thread.        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code.        The name or IP address of the machine on which you want to run the ActiveX server.  The name can be either a UNC name ("\\playdough")  or DNS name ("plato.natinst.com").

If you pass NULL for this parameter, and there is a RemoteServerName registry entry for this server, the server will be run on the machine specified by the RemoteServerName entry.

If you pass NULL for this parameter and there is no RemoteServerName registry entry for this server, the server will be run on the same machine as your program.    �    Pass 0 if you use the object only from the thread that calls this function.  Pass 1 if you use the object from multiple threads.

The CVI ActiveX library uses the COM Global Interface Table (GIT) to marshal interface pointers between threads.  There is overhead associated with using the GIT that should be avoided when possible.  If you do not pass the CAObjHandle between threads in your application, you do not need to use the GIT.    S    Pass the locale for the object.  This value tells the object how to interpret arguments to its methods.  Pass LOCALE_NEUTRAL to indicate the default language-neutral locale.  This value is not used by the server when you call it through a dual interface method.

The CVI ActiveX library passes this value to the IDispatch::Invoke method.     B    This parameter is reserved.  You must pass 0 for this parameter.     �    A handle to the requested ActiveX object.

Use this handle to call methods and get/set properties of this ActiveX object.

When it is no longer needed, you must discard this handle using CA_DiscardObjHandle.    `
����  �    Status                            n -   �  �    Server                            x - �     �    Support Multithreading            	4 -� �  �    Locale                            
� �      �    Reserved                          
� � � �  �    Object Handle                      	           NULL    1    LOCALE_NEUTRAL    0    	          r    Use this function to load an existing IKYF75 object from a file, and obtain a handle to the object.

If the server application is already running, this function may or may not start another copy of the application.  This is determined by the server application.

You must call CA_InitActiveXThreadStyleForCurrentThread with COINIT_APARTMENTTHREADED if you register any ActiveX event callbacks and want the callbacks to be called from the same thread in which they were registered.  If you do not call CA_InitActiveXThreadStyleForCurrentThread with COINIT_APARTMENTTHREADED your callbacks will be called from a system thread.        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code.     :    A file containing the data for an ActiveX server object.        The name or IP address of the machine on which you want to run the ActiveX server.  The name can be either a UNC name ("\\playdough")  or DNS name ("plato.natinst.com").

If you pass NULL for this parameter, and there is an ActivateAtStorage registry entry for this server, the server will be run on the machine on which the file specified by the filename parameter resides.

If you pass NULL for this parameter and there is no ActivateAtStorage registry entry for this server, the server will be run on the same machine as your program.    �    Pass 0 if you use the object only from the thread that calls this function.  Pass 1 if you use the object from multiple threads.

The CVI ActiveX library uses the COM Global Interface Table (GIT) to marshal interface pointers between threads.  There is overhead associated with using the GIT that should be avoided when possible.  If you do not pass the CAObjHandle between threads in your application, you do not need to use the GIT.    S    Pass the locale for the object.  This value tells the object how to interpret arguments to its methods.  Pass LOCALE_NEUTRAL to indicate the default language-neutral locale.  This value is not used by the server when you call it through a dual interface method.

The CVI ActiveX library passes this value to the IDispatch::Invoke method.     B    This parameter is reserved.  You must pass 0 for this parameter.     �    A handle to the requested ActiveX object.

Use this handle to call methods and get/set properties of this ActiveX object.

When it is no longer needed, you must discard this handle using CA_DiscardObjHandle.    �
����  �    Status                            � -   �  �    File Name                         � - � �  �    Server                             -�     �    Support Multithreading            � �  �  �    Locale                            0 � �     �    Reserved                          z �� �  �    Object Handle                      	               NULL    1    LOCALE_NEUTRAL    0    	          �    Use this function to get a handle to an active IKYF75 object.

You must call CA_InitActiveXThreadStyleForCurrentThread with COINIT_APARTMENTTHREADED if you register any ActiveX event callbacks and want the callbacks to be called from the same thread in which they were registered.  If you do not call CA_InitActiveXThreadStyleForCurrentThread with COINIT_APARTMENTTHREADED your callbacks will be called from a system thread.        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code.    �    The name or IP address of the machine on which to look for the active ActiveX server object.  The name can be either a UNC name ("\\playdough")  or DNS name ("plato.natinst.com").

If you pass NULL, the function will look for the active ActiveX server object on the same machine as your program.

NOTE: Windows 95 and Windows NT 4.0 do not support accessing active objects on remote machines.  Future versions of these operating systems may support this functionality.    �    Pass 0 if you use the object only from the thread that calls this function.  Pass 1 if you use the object from multiple threads.

The CVI ActiveX library uses the COM Global Interface Table (GIT) to marshal interface pointers between threads.  There is overhead associated with using the GIT that should be avoided when possible.  If you do not pass the CAObjHandle between threads in your application, you do not need to use the GIT.    S    Pass the locale for the object.  This value tells the object how to interpret arguments to its methods.  Pass LOCALE_NEUTRAL to indicate the default language-neutral locale.  This value is not used by the server when you call it through a dual interface method.

The CVI ActiveX library passes this value to the IDispatch::Invoke method.     B    This parameter is reserved.  You must pass 0 for this parameter.     �    A handle to the requested ActiveX object.

Use this handle to call methods and get/set properties of this ActiveX object.

When it is no longer needed, you must discard this handle using CA_DiscardObjHandle.    �
����  �    Status                            � -   �  �    Server                            � - �     �    Support Multithreading            ` -� �  �    Locale                             � �      �    Reserved                          ! � � �  �    Object Handle                      	           NULL    1    LOCALE_NEUTRAL    0    	               Method Initialize        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    #s
����  �    Status                            %� -   �  �    Object Handle                     &� - � �  �    Error Info                         	                       NULL        Method StartIsoc        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    )�
����  �    Status                            , -   �  �    Object Handle                     -L - � �  �    Error Info                         	                       NULL        Method StopIsoc        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    0P
����  �    Status                            2o -   �  �    Object Handle                     3� - � �  �    Error Info                         	                       NULL        Method GetLiveImage        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    6�
����  �    Status                            8� -   �  �    Object Handle                     :, - � �  �    Error Info                        <X -�    �    Hdib                               	                       NULL    	                Method GetStillImage        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    >'
����  �    Status                            @F -   �  �    Object Handle                     A� - � �  �    Error Info                        C� -�    �    Hdib                               	                       NULL    	                Method FreezeTrigger        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    E�
����  �    Status                            G� -   �  �    Object Handle                     H� - � �  �    Error Info                         	                       NULL        Method RestartShutter        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    L 
����  �    Status                            N -   �  �    Object Handle                     Oj - � �  �    Error Info                         	                       NULL        Method ReadDeviceList        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    6    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.

When it is no longer needed, you must free the string returned in this parameter by calling the CVI ActiveX Library function CA_FreeMemory.    Rt
����  �    Status                            T� -   �  �    Object Handle                     U� - � �  �    Error Info                        X
 -�    �    Index                             Xp �     �    Item                               	                       NULL        	                Method GetMaxValue        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    [
����  �    Status                            ]" -   �  �    Object Handle                     ^m - � �  �    Error Info                        `� -�    �    Item                              `� �     �    Value                              	                       NULL        	                Method GetMinValue        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    c
����  �    Status                            e$ -   �  �    Object Handle                     fo - � �  �    Error Info                        h� -�    �    Item                              i �     �    Value                              	                       NULL        	                Method GetValueText        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    6    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.

When it is no longer needed, you must free the string returned in this parameter by calling the CVI ActiveX Library function CA_FreeMemory.    k
����  �    Status                            m' -   �  �    Object Handle                     nr - � �  �    Error Info                        p� -�    �    Item                              q �     �    Value                              	                       NULL        	                Method LoadFile        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    s�
����  �    Status                            u� -   �  �    Object Handle                     v� - � �  �    Error Info                        y* -� �  �    Filepath                           	                       NULL    ""        Method SaveFile        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    z�
����  �    Status                            |� -   �  �    Object Handle                     ~ - � �  �    Error Info                        �9 -� �  �    Filepath                           	                       NULL    ""        Method AutoWhiteBalance        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    ��
����  �    Status                            �� -   �  �    Object Handle                     �$ - � �  �    Error Info                         	                       NULL        Method PixelCheck        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.    �*
����  �    Status                            �I -   �  �    Object Handle                     �� - � �  �    Error Info                         	                       NULL        Method xGetChromaData        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    ��
����  �    Status                            �� -   �  �    Object Handle                     � - � �  �    Error Info                        �4 -�    �    Passwd                            �� �     �    Targetline                        �  � �    �    Ptr                                	                       NULL            	                Method MemorySave        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    �>
����  �    Status                            �] -   �  �    Object Handle                     �� - � �  �    Error Info                        �� -�    �    Channel                            	                       NULL            Method InqIrisPresence        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    �R
����  �    Status                            �q -   �  �    Object Handle                     �� - � �  �    Error Info                        �� -�    �    P Val                              	                       NULL    	                Method InqFocusPresence        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    ��
����  �    Status                            �� -   �  �    Object Handle                     �$ - � �  �    Error Info                        �P -�    �    P Val                              	                       NULL    	                Method InqZoomPresence        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    �!
����  �    Status                            �@ -   �  �    Object Handle                     �� - � �  �    Error Info                        �� -�    �    P Val                              	                       NULL    	                Method InqIrisAeDetect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    ��
����  �    Status                            �� -   �  �    Object Handle                     �� - � �  �    Error Info                        � -�    �    Value                              	                       NULL    	                Method InqIrisAeLevel        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    ��
����  �    Status                            � -   �  �    Object Handle                     �X - � �  �    Error Info                        �� -�    �    Value                              	                       NULL    	                Method InqAutoWhiteBalance        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    �Y
����  �    Status                            �x -   �  �    Object Handle                     �� - � �  �    Error Info                        �� -�    �    Value                              	                       NULL    	                Method InqRestartShutter        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    ��
����  �    Status                            �� -   �  �    Object Handle                     �, - � �  �    Error Info                        �X -�    �    Value                              	                       NULL    	                Method InqPatternLevel        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    �)
����  �    Status                            �H -   �  �    Object Handle                     Փ - � �  �    Error Info                        ׿ -�    �    P Val                              	                       NULL    	                Method InqPixelCheck        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    َ
����  �    Status                            ۭ -   �  �    Object Handle                     �� - � �  �    Error Info                        �$ -�    �    Value                              	                       NULL    	                Method DrawAeArea        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    ��
����  �    Status                            � -   �  �    Object Handle                     �Z - � �  �    Error Info                        � -�    �    On_off                             	                       NULL            Method xGetWaveformData        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    �
����  �    Status                            �$ -   �  �    Object Handle                     �o - � �  �    Error Info                        � -�    �    Passwd                            � �     �    Targetline                        �g � �    �    Selectline                        �� ��    �    Color                             �3 �     �    Direction                         � � �    �    Ptr                                	                       NULL                        	                Method xGetFirmwareVersion        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    6    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.

When it is no longer needed, you must free the string returned in this parameter by calling the CVI ActiveX Library function CA_FreeMemory.    �
����  �    Status                            �� -   �  �    Object Handle                     �� - � �  �    Error Info                        �! -�    �    Passwd                            �� �     �    Value                              	                       NULL        	                Method xGetLiveFrameRate        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.    6    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.

When it is no longer needed, you must free the string returned in this parameter by calling the CVI ActiveX Library function CA_FreeMemory.    � 
����  �    Status                            �? -   �  �    Object Handle                     �� - � �  �    Error Info                        � -�    �    Passwd                            �     �    Value                              	                       NULL        	                Method xAsyncTransfer        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     ^    Documentation for this function, if provided by the server, is located in the function help.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                     - � �  �    Error Info                       	H -�    �    Passwd                           	� �     �    Quadlet                          
 � �    �    Offset                           
z ��    �    Reg                              
� �     �    Rw                               F � �    �    Value                              	                       NULL                        	                Property DeviceCount        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   2
����  �    Status                           Q -   �  �    Object Handle                    � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property TargetIndex        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                     - � �  �    Error Info                       - -�    �    P Val                              	                       NULL    	                Property TargetIndex        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                            -   �  �    Object Handle                     f - � �  �    Error Info                       "� -�    �    P Val                              	                       NULL            Property LicenceString        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   $
����  �    Status                           &/ -   �  �    Object Handle                    'z - � �  �    Error Info                       )� -� �  �    New Value                          	                       NULL    ""        Property iris_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   +"
����  �    Status                           -A -   �  �    Object Handle                    .� - � �  �    Error Info                       0� -�    �    P Val                              	                       NULL    	                Property iris_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   2�
����  �    Status                           4� -   �  �    Object Handle                    5� - � �  �    Error Info                       8 -�    �    P Val                              	                       NULL            Property iris_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   9�
����  �    Status                           ;� -   �  �    Object Handle                    =  - � �  �    Error Info                       ?, -�    �    P Val                              	                       NULL    	                Property iris_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   @�
����  �    Status                           C -   �  �    Object Handle                    Dd - � �  �    Error Info                       F� -�    �    P Val                              	                       NULL            Property iris_ae_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   H
����  �    Status                           J- -   �  �    Object Handle                    Kx - � �  �    Error Info                       M� -�    �    P Val                              	                       NULL    	                Property iris_ae_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   Ou
����  �    Status                           Q� -   �  �    Object Handle                    R� - � �  �    Error Info                       U -�    �    P Val                              	                       NULL            Property iris_ae_area        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   V�
����  �    Status                           X� -   �  �    Object Handle                    Y� - � �  �    Error Info                       \ -�    �    P Val                              	                       NULL    	                Property iris_ae_area        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ]�
����  �    Status                           ` -   �  �    Object Handle                    aX - � �  �    Error Info                       c� -�    �    P Val                              	                       NULL            Property iris_ae_detect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   e
����  �    Status                           g" -   �  �    Object Handle                    hm - � �  �    Error Info                       j� -�    �    P Val                              	                       NULL    	                Property iris_ae_detect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   lk
����  �    Status                           n� -   �  �    Object Handle                    o� - � �  �    Error Info                       r -�    �    P Val                              	                       NULL            Property shutter_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   s~
����  �    Status                           u� -   �  �    Object Handle                    v� - � �  �    Error Info                       y -�    �    P Val                              	                       NULL    	                Property shutter_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   z�
����  �    Status                           } -   �  �    Object Handle                    ~N - � �  �    Error Info                       �z -�    �    P Val                              	                       NULL            Property shutter_step_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �g - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property shutter_step_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �i
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property shutter_vscan_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property shutter_vscan_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �Z - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property shutter_random_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �* -   �  �    Object Handle                    �u - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property shutter_random_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �y
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property gain_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property gain_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �V - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property gain_step_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �! -   �  �    Object Handle                    �l - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property gain_step_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �k
����  �    Status                           Ŋ -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property gain_vgain_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ʂ
����  �    Status                           ̡ -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property gain_vgain_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �V - � �  �    Error Info                       ׂ -�    �    P Val                              	                       NULL            Property alc_max_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   � 
����  �    Status                           � -   �  �    Object Handle                    �j - � �  �    Error Info                       ޖ -�    �    P Val                              	                       NULL    	                Property alc_max_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �g
����  �    Status                           � -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property eei_limit_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �}
����  �    Status                           � -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property eei_limit_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �P - � �  �    Error Info                       �| -�    �    P Val                              	                       NULL            Property color_temp        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �a - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property color_temp        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �[
����  �    Status                           �z -   �  �    Object Handle                     � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property white_bal_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   p
����  �    Status                           � -   �  �    Object Handle                    � - � �  �    Error Info                       
 -�    �    P Val                              	                       NULL    	                Property white_bal_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                    B - � �  �    Error Info                       n -�    �    P Val                              	                       NULL        !    Property white_bal_auto_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                            -   �  �    Object Handle                    _ - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	            !    Property white_bal_auto_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   e
����  �    Status                           � -   �  �    Object Handle                    � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL        !    Property white_bal_auto_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   !�
����  �    Status                           #� -   �  �    Object Handle                    $� - � �  �    Error Info                       ' -�    �    P Val                              	                       NULL    	            !    Property white_bal_auto_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   (�
����  �    Status                           + -   �  �    Object Handle                    ,\ - � �  �    Error Info                       .� -�    �    P Val                              	                       NULL             Property white_bal_auto_base_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   0
����  �    Status                           2- -   �  �    Object Handle                    3x - � �  �    Error Info                       5� -�    �    P Val                              	                       NULL    	                 Property white_bal_auto_base_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   7}
����  �    Status                           9� -   �  �    Object Handle                    :� - � �  �    Error Info                       = -�    �    P Val                              	                       NULL             Property white_bal_auto_base_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   >�
����  �    Status                           @� -   �  �    Object Handle                    B - � �  �    Error Info                       D/ -�    �    P Val                              	                       NULL    	                 Property white_bal_auto_base_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   F
����  �    Status                           H' -   �  �    Object Handle                    Ir - � �  �    Error Info                       K� -�    �    P Val                              	                       NULL        "    Property white_bal_auto2_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   M&
����  �    Status                           OE -   �  �    Object Handle                    P� - � �  �    Error Info                       R� -�    �    P Val                              	                       NULL    	            "    Property white_bal_auto2_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   T�
����  �    Status                           V� -   �  �    Object Handle                    X - � �  �    Error Info                       Z- -�    �    P Val                              	                       NULL        "    Property white_bal_auto2_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   [�
����  �    Status                           ]� -   �  �    Object Handle                    _ - � �  �    Error Info                       aK -�    �    P Val                              	                       NULL    	            "    Property white_bal_auto2_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   c&
����  �    Status                           eE -   �  �    Object Handle                    f� - � �  �    Error Info                       h� -�    �    P Val                              	                       NULL        !    Property white_bal_auto2_base_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   jC
����  �    Status                           lb -   �  �    Object Handle                    m� - � �  �    Error Info                       o� -�    �    P Val                              	                       NULL    	            !    Property white_bal_auto2_base_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   q�
����  �    Status                           s� -   �  �    Object Handle                    u - � �  �    Error Info                       wI -�    �    P Val                              	                       NULL        !    Property white_bal_auto2_base_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   x�
����  �    Status                           z� -   �  �    Object Handle                    |: - � �  �    Error Info                       ~f -�    �    P Val                              	                       NULL    	            !    Property white_bal_auto2_base_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �@
����  �    Status                           �_ -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL        #    Property white_bal_manual_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �_
����  �    Status                           �~ -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	            #    Property white_bal_manual_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �; - � �  �    Error Info                       �g -�    �    P Val                              	                       NULL        #    Property white_bal_manual_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �Z - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	            #    Property white_bal_manual_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �b
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property shading_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �u
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property shading_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �E - � �  �    Error Info                       �q -�    �    P Val                              	                       NULL            Property shading_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �[ - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property shading_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �Z
����  �    Status                           �y -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property shading_level_g        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �p
����  �    Status                           Ï -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property shading_level_g        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �C - � �  �    Error Info                       �o -�    �    P Val                              	                       NULL            Property shading_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �Y - � �  �    Error Info                       Յ -�    �    P Val                              	                       NULL    	                Property shading_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �X
����  �    Status                           �w -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property detail_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �j
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �  -�    �    P Val                              	                       NULL    	                Property detail_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �9 - � �  �    Error Info                       �e -�    �    P Val                              	                       NULL            Property detail_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �L - � �  �    Error Info                       �x -�    �    P Val                              	                       NULL    	                Property detail_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �H
����  �    Status                           �g -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property detail_level_dep        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �_
����  �    Status                           �~ -   �  �    Object Handle                    �� - � �  �    Error Info                        � -�    �    P Val                              	                       NULL    	                Property detail_level_dep        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                    3 - � �  �    Error Info                       _ -�    �    P Val                              	                       NULL            Property detail_noise_supp        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   	�
����  �    Status                             -   �  �    Object Handle                    K - � �  �    Error Info                       w -�    �    P Val                              	                       NULL    	                Property detail_noise_supp        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   L
����  �    Status                           k -   �  �    Object Handle                    � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property gamma_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ]
����  �    Status                           | -   �  �    Object Handle                    � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property gamma_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           !� -   �  �    Object Handle                    #+ - � �  �    Error Info                       %W -�    �    P Val                              	                       NULL            Property gamma_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   &�
����  �    Status                           (� -   �  �    Object Handle                    *= - � �  �    Error Info                       ,i -�    �    P Val                              	                       NULL    	                Property gamma_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   .8
����  �    Status                           0W -   �  �    Object Handle                    1� - � �  �    Error Info                       3� -�    �    P Val                              	                       NULL            Property nega_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   5H
����  �    Status                           7g -   �  �    Object Handle                    8� - � �  �    Error Info                       :� -�    �    P Val                              	                       NULL    	                Property nega_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   <�
����  �    Status                           >� -   �  �    Object Handle                    @ - � �  �    Error Info                       BA -�    �    P Val                              	                       NULL            Property dsp_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   C�
����  �    Status                           E� -   �  �    Object Handle                    G$ - � �  �    Error Info                       IP -�    �    P Val                              	                       NULL    	                Property dsp_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   K
����  �    Status                           M; -   �  �    Object Handle                    N� - � �  �    Error Info                       P� -�    �    P Val                              	                       NULL            Property master_black_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   R5
����  �    Status                           TT -   �  �    Object Handle                    U� - � �  �    Error Info                       W� -�    �    P Val                              	                       NULL    	                Property master_black_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   Y�
����  �    Status                           [� -   �  �    Object Handle                    ] - � �  �    Error Info                       _7 -�    �    P Val                              	                       NULL            Property flare_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   `�
����  �    Status                           b� -   �  �    Object Handle                    d - � �  �    Error Info                       fH -�    �    P Val                              	                       NULL    	                Property flare_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   h
����  �    Status                           j5 -   �  �    Object Handle                    k� - � �  �    Error Info                       m� -�    �    P Val                              	                       NULL            Property flare_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   o*
����  �    Status                           qI -   �  �    Object Handle                    r� - � �  �    Error Info                       t� -�    �    P Val                              	                       NULL    	                Property flare_level_r        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   v�
����  �    Status                           x� -   �  �    Object Handle                    y� - � �  �    Error Info                       |' -�    �    P Val                              	                       NULL            Property flare_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   }�
����  �    Status                           � -   �  �    Object Handle                    � - � �  �    Error Info                       �; -�    �    P Val                              	                       NULL    	                Property flare_level_b        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �+ -   �  �    Object Handle                    �v - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property abl_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �: -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property abl_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �}
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property abl_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �# -�    �    P Val                              	                       NULL    	                Property abl_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �Z - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property pixel_compen        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �" -   �  �    Object Handle                    �m - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property pixel_compen        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �i
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property color_mat_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �~
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property color_mat_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �P - � �  �    Error Info                       �| -�    �    P Val                              	                       NULL            Property color_mat_level0        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �g - � �  �    Error Info                       ˓ -�    �    P Val                              	                       NULL    	                Property color_mat_level0        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �g
����  �    Status                           φ -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property color_mat_level1        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �~
����  �    Status                           ֝ -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property color_mat_level1        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �R - � �  �    Error Info                       �~ -�    �    P Val                              	                       NULL            Property color_mat_level2        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �i - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property color_mat_level2        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �i
����  �    Status                           � -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property color_mat_level3        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property color_mat_level3        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �	 -   �  �    Object Handle                    �T - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property color_mat_level4        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.    
����  �    Status                             -   �  �    Object Handle                    k - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property color_mat_level4        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   k
����  �    Status                           	� -   �  �    Object Handle                    
� - � �  �    Error Info                        -�    �    P Val                              	                       NULL            Property color_mat_level5        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                    � - � �  �    Error Info                        -�    �    P Val                              	                       NULL    	                Property color_mat_level5        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                            -   �  �    Object Handle                    V - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property color_mat_level6        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   
����  �    Status                           " -   �  �    Object Handle                     m - � �  �    Error Info                       "� -�    �    P Val                              	                       NULL    	                Property color_mat_level6        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   $m
����  �    Status                           &� -   �  �    Object Handle                    '� - � �  �    Error Info                       * -�    �    P Val                              	                       NULL            Property color_mat_level7        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   +�
����  �    Status                           -� -   �  �    Object Handle                    .� - � �  �    Error Info                       1 -�    �    P Val                              	                       NULL    	                Property color_mat_level7        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   2�
����  �    Status                           5 -   �  �    Object Handle                    6X - � �  �    Error Info                       8� -�    �    P Val                              	                       NULL            Property color_mat_level8        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   :
����  �    Status                           <$ -   �  �    Object Handle                    =o - � �  �    Error Info                       ?� -�    �    P Val                              	                       NULL    	                Property color_mat_level8        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   Ao
����  �    Status                           C� -   �  �    Object Handle                    D� - � �  �    Error Info                       G -�    �    P Val                              	                       NULL            Property lens_focus_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   H�
����  �    Status                           J� -   �  �    Object Handle                    K� - � �  �    Error Info                       N -�    �    P Val                              	                       NULL    	                Property lens_focus_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   O�
����  �    Status                           R -   �  �    Object Handle                    SZ - � �  �    Error Info                       U� -�    �    P Val                              	                       NULL            Property lens_zoom_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   W
����  �    Status                           Y% -   �  �    Object Handle                    Zp - � �  �    Error Info                       \� -�    �    P Val                              	                       NULL    	                Property lens_zoom_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ^o
����  �    Status                           `� -   �  �    Object Handle                    a� - � �  �    Error Info                       d -�    �    P Val                              	                       NULL            Property freeze_cancel_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   e�
����  �    Status                           g� -   �  �    Object Handle                    h� - � �  �    Error Info                       k -�    �    P Val                              	                       NULL    	                Property freeze_cancel_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   l�
����  �    Status                           o -   �  �    Object Handle                    p^ - � �  �    Error Info                       r� -�    �    P Val                              	                       NULL            Property test_pattern_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   t
����  �    Status                           v+ -   �  �    Object Handle                    wv - � �  �    Error Info                       y� -�    �    P Val                              	                       NULL    	                Property test_pattern_mode        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   {w
����  �    Status                           }� -   �  �    Object Handle                    ~� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property test_pattern_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �& -�    �    P Val                              	                       NULL    	                Property test_pattern_level        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �f - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property still_color        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �- -   �  �    Object Handle                    �x - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property still_color        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �s
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �	 -�    �    P Val                              	                       NULL            Property still_step        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property still_step        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �R - � �  �    Error Info                       �~ -�    �    P Val                              	                       NULL            Property still_aspect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �e - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property still_aspect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �a
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property still_bytes        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �s
����  �    Status                           �� -   �  �    Object Handle                    �� - � �  �    Error Info                       �	 -�    �    P Val                              	                       NULL    	                Property still_height        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           �� -   �  �    Object Handle                    �C - � �  �    Error Info                       �o -�    �    P Val                              	                       NULL    	                Property still_width        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �>
����  �    Status                           �] -   �  �    Object Handle                    Ψ - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL    	                Property live_color        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   Ң
����  �    Status                           �� -   �  �    Object Handle                    � - � �  �    Error Info                       �8 -�    �    P Val                              	                       NULL    	                Property live_color        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �% -   �  �    Object Handle                    �p - � �  �    Error Info                       ߜ -�    �    P Val                              	                       NULL            Property live_step        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           �5 -   �  �    Object Handle                    � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property live_step        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   �y
����  �    Status                           � -   �  �    Object Handle                    �� - � �  �    Error Info                       � -�    �    P Val                              	                       NULL            Property live_aspect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                    �� - � �  �    Error Info                       �! -�    �    P Val                              	                       NULL    	                Property live_aspect        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     ^    Documentation for this function, if provided by the server, is located in the function help.   ��
����  �    Status                           � -   �  �    Object Handle                    �Z - � �  �    Error Info                       �� -�    �    P Val                              	                       NULL            Property live_bytes        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                              -   �  �    Object Handle                    k - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	                Property live_height        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   f
����  �    Status                           � -   �  �    Object Handle                    � - � �  �    Error Info                       
� -�    �    P Val                              	                       NULL    	                Property live_width        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   �
����  �    Status                           � -   �  �    Object Handle                    4 - � �  �    Error Info                       ` -�    �    P Val                              	                       NULL    	                Property freeze_status        A value indicating whether an error occurred.  A negative error code indicates function failure.

Error codes are defined in cvi\include\cviauto.h and cvi\sdk\include\winerror.h.

You can use CA_GetAutomationErrorString to get the description of an error code or CA_DisplayErrorInfo to display the description of the error code.

If the error code is DISP_E_EXCEPTION  (0x80020009 or -2147352567), then the Error Info parameter contains additional error information.  You can use CA_DisplayErrorInfo to display the error information.    C    An ActiveX object handle obtained from NewIKYF75, OpenIKYF75, ActiveIKYF75, or an ActiveX method or property.

All of the methods that can be applied to a particular object are grouped under a single class in the function tree.  The name of the class corresponds to the type of the object to which this handle must refer.    $    When an ActiveX server function fails with the error code DISP_E_EXCEPTION, descriptive information about the error code is stored in this parameter.  The descriptive information may include the error's code, source, and description.  It may also include a help file and help file context.

When an ActiveX server function fails with the error codes DISP_E_PARAMNOTFOUND, DISP_E_TYPEMISMATCH, or E_INVALIDARG, the parameter position of the invalid argument may be stored in the errorParamPos member of this parameter.

This parameter may be NULL.     �    You can pass NULL for this parameter if you do not need this information.

Documentation for this function, if provided by the server, is located in the function help.   1
����  �    Status                           P -   �  �    Object Handle                    � - � �  �    Error Info                       � -�    �    P Val                              	                       NULL    	         ����         �  �             K.        NewIKYF75                                                                                                                               ����         ,  S             K.        OpenIKYF75                                                                                                                              ����           !�             K.        ActiveIKYF75                                                                                                                            ����         #X  )	             K.        IKYF75Initialize                                                                                                                        ����         )�  /x             K.        IKYF75StartIsoc                                                                                                                         ����         07  5�             K.        IKYF75StopIsoc                                                                                                                          ����         6�  =	             K.        IKYF75GetLiveImage                                                                                                                      ����         >	  Dn             K.        IKYF75GetStillImage                                                                                                                     ����         En  K"             K.        IKYF75FreezeTrigger                                                                                                                     ����         K�  Q�             K.        IKYF75RestartShutter                                                                                                                    ����         RU  Y�             K.        IKYF75ReadDeviceList                                                                                                                    ����         Z�  a�             K.        IKYF75GetMaxValue                                                                                                                       ����         b�  i�             K.        IKYF75GetMinValue                                                                                                                       ����         j�  rB             K.        IKYF75GetValueText                                                                                                                      ����         s{  y�             K.        IKYF75LoadFile                                                                                                                          ����         z�  ��             K.        IKYF75SaveFile                                                                                                                          ����         ��  �P             K.        IKYF75AutoWhiteBalance                                                                                                                  ����         �  ��             K.        IKYF75PixelCheck                                                                                                                        ����         �  ��             K.        IKYF75xGetChromaData                                                                                                                    ����         �#  �:             K.        IKYF75MemorySave                                                                                                                        ����         �2  ��             K.        IKYF75InqIrisPresence                                                                                                                   ����         ��  �             K.        IKYF75InqFocusPresence                                                                                                                  ����         �  �h             K.        IKYF75InqZoomPresence                                                                                                                   ����         �h  ��             K.        IKYF75InqIrisAeDetect                                                                                                                   ����         ��  �5             K.        IKYF75InqIrisAeLevel                                                                                                                    ����         �5  ɠ             K.        IKYF75InqAutoWhiteBalance                                                                                                               ����         ʠ  �	             K.        IKYF75InqRestartShutter                                                                                                                 ����         �	  �p             K.        IKYF75InqPatternLevel                                                                                                                   ����         �p  ��             K.        IKYF75InqPixelCheck                                                                                                                     ����         ��  ��             K.        IKYF75DrawAeArea                                                                                                                        ����         ��  �J         	    K.        IKYF75xGetWaveformData                                                                                                                  ����         �g  ��             K.        IKYF75xGetFirmwareVersion                                                                                                               ����         �� Z             K.        IKYF75xGetLiveFrameRate                                                                                                                 ����        � �         	    K.        IKYF75xAsyncTransfer                                                                                                                    ����         y             K.        IKYF75GetDeviceCount                                                                                                                    ����        y �             K.        IKYF75GetTargetIndex                                                                                                                    ����        � "�             K.        IKYF75SetTargetIndex                                                                                                                    ����        #� *             K.        IKYF75SetLicenceString                                                                                                                  ����        + 1i             K.        IKYF75Getiris_mode                                                                                                                      ����        2i 8�             K.        IKYF75Setiris_mode                                                                                                                      ����        9y ?�             K.        IKYF75Getiris_level                                                                                                                     ����        @� F�             K.        IKYF75Setiris_level                                                                                                                     ����        G� NU             K.        IKYF75Getiris_ae_level                                                                                                                  ����        OU Uq             K.        IKYF75Setiris_ae_level                                                                                                                  ����        Vi \�             K.        IKYF75Getiris_ae_area                                                                                                                   ����        ]� c�             K.        IKYF75Setiris_ae_area                                                                                                                   ����        d� kJ             K.        IKYF75Getiris_ae_detect                                                                                                                 ����        lJ rg             K.        IKYF75Setiris_ae_detect                                                                                                                 ����        s_ y�             K.        IKYF75Getshutter_mode                                                                                                                   ����        z� ��             K.        IKYF75Setshutter_mode                                                                                                                   ����        �� �D             K.        IKYF75Getshutter_step_level                                                                                                             ����        �D �e             K.        IKYF75Setshutter_step_level                                                                                                             ����        �] ��             K.        IKYF75Getshutter_vscan_level                                                                                                            ����        �� ��             K.        IKYF75Setshutter_vscan_level                                                                                                            ����        �� �R             K.        IKYF75Getshutter_random_level                                                                                                           ����        �R �u             K.        IKYF75Setshutter_random_level                                                                                                           ����        �m ��             K.        IKYF75Getgain_mode                                                                                                                      ����        �� ��             K.        IKYF75Setgain_mode                                                                                                                      ����        �� �I             K.        IKYF75Getgain_step_level                                                                                                                ����        �I �g             K.        IKYF75Setgain_step_level                                                                                                                ����        �_ ��             K.        IKYF75Getgain_vgain_level                                                                                                               ����        �� ��             K.        IKYF75Setgain_vgain_level                                                                                                               ����        �� �G             K.        IKYF75Getalc_max_level                                                                                                                  ����        �G �c             K.        IKYF75Setalc_max_level                                                                                                                  ����        �[ ��             K.        IKYF75Geteei_limit_level                                                                                                                ����        �� ��             K.        IKYF75Seteei_limit_level                                                                                                                ����        �� �>             K.        IKYF75Getcolor_temp                                                                                                                     ����        �> W             K.        IKYF75Setcolor_temp                                                                                                                     ����        O 
�             K.        IKYF75Getwhite_bal_mode                                                                                                                 ����        � �             K.        IKYF75Setwhite_bal_mode                                                                                                                 ����        � <             K.        IKYF75Getwhite_bal_auto_level_r                                                                                                         ����        <  a             K.        IKYF75Setwhite_bal_auto_level_r                                                                                                         ����        !Y '�             K.        IKYF75Getwhite_bal_auto_level_b                                                                                                         ����        (� .�             K.        IKYF75Setwhite_bal_auto_level_b                                                                                                         ����        /� 6U             K.        IKYF75Getwhite_bal_auto_base_r                                                                                                          ����        7U =y             K.        IKYF75Setwhite_bal_auto_base_r                                                                                                          ����        >q D�             K.        IKYF75Getwhite_bal_auto_base_b                                                                                                          ����        E� L             K.        IKYF75Setwhite_bal_auto_base_b                                                                                                          ����        L� Sm             K.        IKYF75Getwhite_bal_auto2_level_r                                                                                                        ����        Tm Z�             K.        IKYF75Setwhite_bal_auto2_level_r                                                                                                        ����        [� a�             K.        IKYF75Getwhite_bal_auto2_level_b                                                                                                        ����        b� i"             K.        IKYF75Setwhite_bal_auto2_level_b                                                                                                        ����        j p�             K.        IKYF75Getwhite_bal_auto2_base_r                                                                                                         ����        q� w�             K.        IKYF75Setwhite_bal_auto2_base_r                                                                                                         ����        x�              K.        IKYF75Getwhite_bal_auto2_base_b                                                                                                         ����        � �<             K.        IKYF75Setwhite_bal_auto2_base_b                                                                                                         ����        �4 ��             K.        IKYF75Getwhite_bal_manual_level_r                                                                                                       ����        �� ��             K.        IKYF75Setwhite_bal_manual_level_r                                                                                                       ����        �� �7             K.        IKYF75Getwhite_bal_manual_level_b                                                                                                       ����        �7 �^             K.        IKYF75Setwhite_bal_manual_level_b                                                                                                       ����        �V ��             K.        IKYF75Getshading_mode                                                                                                                   ����        �� ��             K.        IKYF75Setshading_mode                                                                                                                   ����        �� �8             K.        IKYF75Getshading_level_r                                                                                                                ����        �8 �V             K.        IKYF75Setshading_level_r                                                                                                                ����        �N Ƿ             K.        IKYF75Getshading_level_g                                                                                                                ����        ȷ ��             K.        IKYF75Setshading_level_g                                                                                                                ����        �� �6             K.        IKYF75Getshading_level_b                                                                                                                ����        �6 �T             K.        IKYF75Setshading_level_b                                                                                                                ����        �L �             K.        IKYF75Getdetail_mode                                                                                                                    ����        � ��             K.        IKYF75Setdetail_mode                                                                                                                    ����        �� �)             K.        IKYF75Getdetail_level                                                                                                                   ����        �) �D             K.        IKYF75Setdetail_level                                                                                                                   ����        �< �             K.        IKYF75Getdetail_level_dep                                                                                                               ����        � �             K.        IKYF75Setdetail_level_dep                                                                                                               ����        	� (             K.        IKYF75Getdetail_noise_supp                                                                                                              ����        ( H             K.        IKYF75Setdetail_noise_supp                                                                                                              ����        @ �             K.        IKYF75Getgamma_mode                                                                                                                     ����        � %�             K.        IKYF75Setgamma_mode                                                                                                                     ����        &� -             K.        IKYF75Getgamma_level                                                                                                                    ����        . 44             K.        IKYF75Setgamma_level                                                                                                                    ����        5, ;�             K.        IKYF75Getnega_mode                                                                                                                      ����        <� B�             K.        IKYF75Setnega_mode                                                                                                                      ����        C� J             K.        IKYF75Getdsp_mode                                                                                                                       ����        K Q             K.        IKYF75Setdsp_mode                                                                                                                       ����        R X|             K.        IKYF75Getmaster_black_level                                                                                                             ����        Y| _�             K.        IKYF75Setmaster_black_level                                                                                                             ����        `� f�             K.        IKYF75Getflare_mode                                                                                                                     ����        g� n             K.        IKYF75Setflare_mode                                                                                                                     ����        o
 uq             K.        IKYF75Getflare_level_r                                                                                                                  ����        vq |�             K.        IKYF75Setflare_level_r                                                                                                                  ����        }� ��             K.        IKYF75Getflare_level_b                                                                                                                  ����        �� �             K.        IKYF75Setflare_level_b                                                                                                                  ����        �  �b             K.        IKYF75Getabl_mode                                                                                                                       ����        �b �y             K.        IKYF75Setabl_mode                                                                                                                       ����        �q ��             K.        IKYF75Getabl_level                                                                                                                      ����        �� ��             K.        IKYF75Setabl_level                                                                                                                      ����        �� �J             K.        IKYF75Getpixel_compen                                                                                                                   ����        �J �e             K.        IKYF75Setpixel_compen                                                                                                                   ����        �] ��             K.        IKYF75Getcolor_mat_mode                                                                                                                 ����        �� ��             K.        IKYF75Setcolor_mat_mode                                                                                                                 ����        �� �D             K.        IKYF75Getcolor_mat_level0                                                                                                               ����        �D �c             K.        IKYF75Setcolor_mat_level0                                                                                                               ����        �[ ��             K.        IKYF75Getcolor_mat_level1                                                                                                               ����        �� ��             K.        IKYF75Setcolor_mat_level1                                                                                                               ����        �� �F             K.        IKYF75Getcolor_mat_level2                                                                                                               ����        �F �e             K.        IKYF75Setcolor_mat_level2                                                                                                               ����        �] ��             K.        IKYF75Getcolor_mat_level3                                                                                                               ����        �� ��             K.        IKYF75Setcolor_mat_level3                                                                                                               ����        �� H             K.        IKYF75Getcolor_mat_level4                                                                                                               ����        H g             K.        IKYF75Setcolor_mat_level4                                                                                                               ����        _ �             K.        IKYF75Getcolor_mat_level5                                                                                                               ����        � �             K.        IKYF75Setcolor_mat_level5                                                                                                               ����        � #J             K.        IKYF75Getcolor_mat_level6                                                                                                               ����        $J *i             K.        IKYF75Setcolor_mat_level6                                                                                                               ����        +a 1�             K.        IKYF75Getcolor_mat_level7                                                                                                               ����        2� 8�             K.        IKYF75Setcolor_mat_level7                                                                                                               ����        9� @L             K.        IKYF75Getcolor_mat_level8                                                                                                               ����        AL Gk             K.        IKYF75Setcolor_mat_level8                                                                                                               ����        Hc N�             K.        IKYF75Getlens_focus_level                                                                                                               ����        O� U�             K.        IKYF75Setlens_focus_level                                                                                                               ����        V� ]M             K.        IKYF75Getlens_zoom_level                                                                                                                ����        ^M dk             K.        IKYF75Setlens_zoom_level                                                                                                                ����        ec k�             K.        IKYF75Getfreeze_cancel_mode                                                                                                             ����        l� r�             K.        IKYF75Setfreeze_cancel_mode                                                                                                             ����        s� zS             K.        IKYF75Gettest_pattern_mode                                                                                                              ����        {S �s             K.        IKYF75Settest_pattern_mode                                                                                                              ����        �k ��             K.        IKYF75Gettest_pattern_level                                                                                                             ����        �� ��             K.        IKYF75Settest_pattern_level                                                                                                             ����        �� �U             K.        IKYF75Getstill_color                                                                                                                    ����        �U �o             K.        IKYF75Setstill_color                                                                                                                    ����        �g ��             K.        IKYF75Getstill_step                                                                                                                     ����        �� ��             K.        IKYF75Setstill_step                                                                                                                     ����        �� �B             K.        IKYF75Getstill_aspect                                                                                                                   ����        �B �]             K.        IKYF75Setstill_aspect                                                                                                                   ����        �U º             K.        IKYF75Getstill_bytes                                                                                                                    ����        ú �              K.        IKYF75Getstill_height                                                                                                                   ����        �  х             K.        IKYF75Getstill_width                                                                                                                    ����        ҅ ��             K.        IKYF75Getlive_color                                                                                                                     ����        �� �             K.        IKYF75Setlive_color                                                                                                                     ����        �� �]             K.        IKYF75Getlive_step                                                                                                                      ����        �] �u             K.        IKYF75Setlive_step                                                                                                                      ����        �m ��             K.        IKYF75Getlive_aspect                                                                                                                    ����        �� ��             K.        IKYF75Setlive_aspect                                                                                                                    ����        �� H             K.        IKYF75Getlive_bytes                                                                                                                     ����        H �             K.        IKYF75Getlive_height                                                                                                                    ����        �              K.        IKYF75Getlive_width                                                                                                                     ����         x             K.        IKYF75Getfreeze_status                                                                                                                        �                                                                                    �IKYF75                                                                               �New IKYF75                                                                           �Open IKYF75                                                                          �Active IKYF75                                                                        �Initialize                                                                           �Start Isoc                                                                           �Stop Isoc                                                                            �Get Live Image                                                                       �Get Still Image                                                                      �Freeze Trigger                                                                       �Restart Shutter                                                                      �Read Device List                                                                     �Get Max Value                                                                        �Get Min Value                                                                        �Get Value Text                                                                       �Load File                                                                            �Save File                                                                            �Auto White Balance                                                                   �Pixel Check                                                                          �X Get Chroma Data                                                                    �Memory Save                                                                          �Inq Iris Presence                                                                    �Inq Focus Presence                                                                   �Inq Zoom Presence                                                                    �Inq Iris Ae Detect                                                                   �Inq Iris Ae Level                                                                    �Inq Auto White Balance                                                               �Inq Restart Shutter                                                                  �Inq Pattern Level                                                                    �Inq Pixel Check                                                                      �Draw Ae Area                                                                         �X Get Waveform Data                                                                  �X Get Firmware Version                                                               �X Get Live Frame Rate                                                                �X Async Transfer                                                                     �Get Device Count                                                                     �Get Target Index                                                                     �Set Target Index                                                                     �Set Licence String                                                                   �Getiris_mode                                                                         �Setiris_mode                                                                         �Getiris_level                                                                        �Setiris_level                                                                        �Getiris_ae_level                                                                     �Setiris_ae_level                                                                     �Getiris_ae_area                                                                      �Setiris_ae_area                                                                      �Getiris_ae_detect                                                                    �Setiris_ae_detect                                                                    �Getshutter_mode                                                                      �Setshutter_mode                                                                      �Getshutter_step_level                                                                �Setshutter_step_level                                                                �Getshutter_vscan_level                                                               �Setshutter_vscan_level                                                               �Getshutter_random_level                                                              �Setshutter_random_level                                                              �Getgain_mode                                                                         �Setgain_mode                                                                         �Getgain_step_level                                                                   �Setgain_step_level                                                                   �Getgain_vgain_level                                                                  �Setgain_vgain_level                                                                  �Getalc_max_level                                                                     �Setalc_max_level                                                                     �Geteei_limit_level                                                                   �Seteei_limit_level                                                                   �Getcolor_temp                                                                        �Setcolor_temp                                                                        �Getwhite_bal_mode                                                                    �Setwhite_bal_mode                                                                    �Getwhite_bal_auto_level_r                                                            �Setwhite_bal_auto_level_r                                                            �Getwhite_bal_auto_level_b                                                            �Setwhite_bal_auto_level_b                                                            �Getwhite_bal_auto_base_r                                                             �Setwhite_bal_auto_base_r                                                             �Getwhite_bal_auto_base_b                                                             �Setwhite_bal_auto_base_b                                                             �Getwhite_bal_auto2_level_r                                                           �Setwhite_bal_auto2_level_r                                                           �Getwhite_bal_auto2_level_b                                                           �Setwhite_bal_auto2_level_b                                                           �Getwhite_bal_auto2_base_r                                                            �Setwhite_bal_auto2_base_r                                                            �Getwhite_bal_auto2_base_b                                                            �Setwhite_bal_auto2_base_b                                                            �Getwhite_bal_manual_level_r                                                          �Setwhite_bal_manual_level_r                                                          �Getwhite_bal_manual_level_b                                                          �Setwhite_bal_manual_level_b                                                          �Getshading_mode                                                                      �Setshading_mode                                                                      �Getshading_level_r                                                                   �Setshading_level_r                                                                   �Getshading_level_g                                                                   �Setshading_level_g                                                                   �Getshading_level_b                                                                   �Setshading_level_b                                                                   �Getdetail_mode                                                                       �Setdetail_mode                                                                       �Getdetail_level                                                                      �Setdetail_level                                                                      �Getdetail_level_dep                                                                  �Setdetail_level_dep                                                                  �Getdetail_noise_supp                                                                 �Setdetail_noise_supp                                                                 �Getgamma_mode                                                                        �Setgamma_mode                                                                        �Getgamma_level                                                                       �Setgamma_level                                                                       �Getnega_mode                                                                         �Setnega_mode                                                                         �Getdsp_mode                                                                          �Setdsp_mode                                                                          �Getmaster_black_level                                                                �Setmaster_black_level                                                                �Getflare_mode                                                                        �Setflare_mode                                                                        �Getflare_level_r                                                                     �Setflare_level_r                                                                     �Getflare_level_b                                                                     �Setflare_level_b                                                                     �Getabl_mode                                                                          �Setabl_mode                                                                          �Getabl_level                                                                         �Setabl_level                                                                         �Getpixel_compen                                                                      �Setpixel_compen                                                                      �Getcolor_mat_mode                                                                    �Setcolor_mat_mode                                                                    �Getcolor_mat_level0                                                                  �Setcolor_mat_level0                                                                  �Getcolor_mat_level1                                                                  �Setcolor_mat_level1                                                                  �Getcolor_mat_level2                                                                  �Setcolor_mat_level2                                                                  �Getcolor_mat_level3                                                                  �Setcolor_mat_level3                                                                  �Getcolor_mat_level4                                                                  �Setcolor_mat_level4                                                                  �Getcolor_mat_level5                                                                  �Setcolor_mat_level5                                                                  �Getcolor_mat_level6                                                                  �Setcolor_mat_level6                                                                  �Getcolor_mat_level7                                                                  �Setcolor_mat_level7                                                                  �Getcolor_mat_level8                                                                  �Setcolor_mat_level8                                                                  �Getlens_focus_level                                                                  �Setlens_focus_level                                                                  �Getlens_zoom_level                                                                   �Setlens_zoom_level                                                                   �Getfreeze_cancel_mode                                                                �Setfreeze_cancel_mode                                                                �Gettest_pattern_mode                                                                 �Settest_pattern_mode                                                                 �Gettest_pattern_level                                                                �Settest_pattern_level                                                                �Getstill_color                                                                       �Setstill_color                                                                       �Getstill_step                                                                        �Setstill_step                                                                        �Getstill_aspect                                                                      �Setstill_aspect                                                                      �Getstill_bytes                                                                       �Getstill_height                                                                      �Getstill_width                                                                       �Getlive_color                                                                        �Setlive_color                                                                        �Getlive_step                                                                         �Setlive_step                                                                         �Getlive_aspect                                                                       �Setlive_aspect                                                                       �Getlive_bytes                                                                        �Getlive_height                                                                       �Getlive_width                                                                        �Getfreeze_status                                                                