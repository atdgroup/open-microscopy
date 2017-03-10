#For help use help(object)
#print help(UIController)
#print help(Panel)

panel = ui.GetPanelByTitle("Test Pan")

# This is the lowlevel way of calling controls - callbacks are called - shouldn't really be used.
panel.OperateControl("string", "Hello")
panel.OperateControl("numeric", 7.54)
panel.OperateControl ("numeric2", 6.265)
panel.OperateControl("TabNumeric1", 2.35)
panel.OperateControl("TabNumeric2", 1.485)
panel.OperateControl("numeric")
panel.OperateControl("OK")

# Higher level way -- needs extending
panel.PressButton("OK")
panel.SetNumeric("numeric", 4.38)
panel.ToggleButtonOn("toggle button")
panel.SelectMenuItem ("File//Open")
panel.SetSliderToValue("Slider Control", -2)
panel.SetSliderToPercentage("Slider Control", 1)

panel.SetListToLabel("string ring", "Value2")
panel.SetListToLabel("int ring", "Value1")
panel.SetListToLabel("List", "List Item 3")

# Project specific extensions
test.testFunction("Hello")
