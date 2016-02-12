' The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

Imports Windows.Graphics.Imaging
Imports Windows.Storage
Imports Windows.Storage.Streams
Imports Windows.UI.Xaml.Media.Animation
''' <summary>
''' An empty page that can be used on its own or navigated to within a Frame.
''' </summary>
Public NotInheritable Class MainPage
    Inherits Page
    Dim WithEvents entryStoryBoard As New Windows.UI.Xaml.Media.Animation.Storyboard
    Dim WithEvents exitStoryBoard As New Windows.UI.Xaml.Media.Animation.Storyboard
    Dim panX As DoubleAnimation
    Dim panY As DoubleAnimation
    Dim panTrans As TranslateTransform
    Dim scaleAnimatiionX As DoubleAnimation
    Dim scaleAnimatiionY As DoubleAnimation
    Dim scaleTrans As ScaleTransform
    Private Sub MainPage_Loaded(sender As Object, e As RoutedEventArgs) Handles Me.Loaded
        loadImage() '
    End Sub
    Public Async Sub loadImage()
        'Try

        Dim ctr As Int16 = 1

        LayoutRoot.Resources.Add("st", entryStoryBoard)
        LayoutRoot.Resources.Add("st1", exitStoryBoard)

        While True
            Try

                Dim str As StorageFile = Await Windows.ApplicationModel.Package.Current.InstalledLocation.GetFileAsync("Assets\IMG_0435.Jpg")

                Dim b As New BitmapImage


                Dim fileStream As IRandomAccessStream = Await str.OpenAsync(FileAccessMode.Read)
                Dim decoder As BitmapDecoder = Await BitmapDecoder.CreateAsync(fileStream)


                Await b.SetSourceAsync(fileStream)



                image.Source = b

                If ctr > 1 Then
                    entryStoryBoard.Stop()
                End If

                Dim Width, Height As Double

                Width = decoder.OrientedPixelWidth
                Height = decoder.OrientedPixelHeight
                Dim outZoomFactor As Point
                Dim imageBoxSize As Size = getImageBoxSize(New Size(Width, Height), outZoomFactor)
                Dim centrePoint As Point
                Dim dnd As New Random


                Dim imgCentre As New Point(imageBoxSize.Width / 2, imageBoxSize.Height / 2)
                Dim rectPerc As Single = 0.3

                    Dim xFrom, xTo, yFrom, yTo As Single

                    xFrom = imgCentre.X - (imageBoxSize.Width * rectPerc)
                    xTo = imgCentre.X + (imageBoxSize.Width * rectPerc)

                    yFrom = imgCentre.Y - (imageBoxSize.Height * rectPerc)
                    yTo = imgCentre.Y + (imageBoxSize.Height * rectPerc)


                    Dim rndX As Integer = dnd.Next(xFrom, xTo)
                    Dim rndY As Integer = dnd.Next(yFrom, yTo)

                    centrePoint.X = rndX
                    centrePoint.Y = rndY

                    Dim imageRatio As Single = imageBoxSize.Width / imageBoxSize.Height
                If imageRatio > 1 Then
                    centrePoint.Y = imgCentre.Y
                Else
                    centrePoint.X = imgCentre.X
                End If

                If outZoomFactor = New Point(1, 1) Then
                    outZoomFactor = New Point(0, 0)
                End If

                Await AnimateUsingKeyFrame(False, centrePoint.X, centrePoint.Y, str.Path, imageBoxSize.Width / 2, imageBoxSize.Height / 2, outZoomFactor.X, outZoomFactor.Y)





                ctr += 1
            Catch ex As Exception

            End Try


        End While


    End Sub
    Public Function getImageBoxSize(PictureSize As Size, ByRef ZoomFactor As Point) As Size
        Dim maxSize As Size = New Size(Me.ActualWidth, Me.ActualHeight)
        Dim resizeReqd As Boolean = False
        Dim rtnSize As Size
        ZoomFactor = New Point(1, 1)
        If PictureSize.Height > maxSize.Height Or PictureSize.Width > maxSize.Width Then

            Dim ratio As Single = PictureSize.Width / PictureSize.Height


            Dim isImageLandScape As Boolean = False
            If ratio > 1 Then
                isImageLandScape = True
            End If


            If PictureSize.Height > maxSize.Height Then
                rtnSize.Height = maxSize.Height
                rtnSize.Width = rtnSize.Height * ratio
                ZoomFactor.Y = maxSize.Height / rtnSize.Height
            ElseIf PictureSize.Width > maxSize.Width Then
                rtnSize.Width = maxSize.Width
                rtnSize.Height = rtnSize.Width * ratio

                ZoomFactor.Y = maxSize.Width / rtnSize.Width
            End If
            ZoomFactor.X = PictureSize.Width / rtnSize.Width
            ZoomFactor.X *= 2
            'If ratio >= 1 Then   ''''' Picture is Landscape or Square

            'Else '''''' Picture is Portrait

            'End If

            'If isImageLandScape = False And rtnSize.Height < rtnSize.Width Then  '''''width > Height
            '    Dim test As Double = rtnSize.Width
            '    rtnSize.Width = rtnSize.Height
            '    rtnSize.Height = test
            'End If



            Return rtnSize
        Else
            Return PictureSize
        End If

    End Function
    Dim processNext As Boolean = False

    Public Async Function AnimateUsingKeyFrame(FadeIn As Boolean, faceCentreX As Single, faceCentreY As Single, filename As String, ImageCentreX As Single, ImageCentreY As Single, Optional ZoomStart As Single = 0, Optional ZoomEnd As Single = 0) As Task
        Dim PanZoomDurationSec As Integer = 10
        Dim ImageStillDisplaySec As Integer = 1
        Dim ImageFadeInSec As Integer = 5
        Dim fadeOutSec As Integer = 3

        panX = New DoubleAnimation
        panY = New DoubleAnimation
        panTrans = New TranslateTransform
        scaleAnimatiionX = New DoubleAnimation
        scaleAnimatiionY = New DoubleAnimation
        scaleTrans = New ScaleTransform



        entryStoryBoard.Children.Clear()
        exitStoryBoard.Children.Clear()


        overlayRect.Opacity = 0
        Dim myDoubleAnimationX As DoubleAnimationUsingKeyFrames = New DoubleAnimationUsingKeyFrames
        myDoubleAnimationX.BeginTime = TimeSpan.FromSeconds(PanZoomDurationSec)
        myDoubleAnimationX.Duration = New Duration(TimeSpan.FromSeconds(ImageStillDisplaySec))
        myDoubleAnimationX.FillBehavior = FillBehavior.HoldEnd

        If ZoomStart = 0 Then ZoomStart = 1
        If ZoomEnd = 0 Then ZoomEnd = 2


        Dim imageExitAnimation As New DoubleAnimationUsingKeyFrames
        imageExitAnimation.BeginTime = TimeSpan.FromSeconds(PanZoomDurationSec + ImageStillDisplaySec + 2)

        Dim imageLoadAnimation As New DoubleAnimation
        imageLoadAnimation.From = 0
        imageLoadAnimation.To = 1
        imageLoadAnimation.Duration = New Duration(TimeSpan.FromSeconds(fadeOutSec))




        Dim easingFunction As EasingFunctionBase = Nothing

        easingFunction = New CubicEase
        easingFunction.EasingMode = EasingMode.EaseIn




        'scaleAnimatiionX.BeginTime = TimeSpan.FromSeconds(fadeOutSec + 3)
        'scaleAnimatiionY.BeginTime = TimeSpan.FromSeconds(fadeOutSec + 3)
        scaleAnimatiionX.Duration = New Duration(TimeSpan.FromSeconds(PanZoomDurationSec))
        scaleAnimatiionY.Duration = New Duration(TimeSpan.FromSeconds(PanZoomDurationSec))



        ' ZoomEnd = ZoomStart

        scaleAnimatiionX.From = ZoomStart
        scaleAnimatiionX.To = ZoomEnd
        scaleAnimatiionX.EasingFunction = easingFunction

        scaleAnimatiionY.From = ZoomStart
        scaleAnimatiionY.To = ZoomEnd
        scaleAnimatiionY.EasingFunction = easingFunction


        'Dim ldkfX As New LinearDoubleKeyFrame

        'ldkfX.Value = 2
        'ldkfX.KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromSeconds(PanZoomDurationSec))
        'scaleAnimatiionX.KeyFrames.Add(ldkfX)

        'Dim ldkfY As New LinearDoubleKeyFrame
        'ldkfY.Value = 2
        'ldkfY.KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromSeconds(PanZoomDurationSec))
        'scaleAnimatiionY.KeyFrames.Add(ldkfY)

        Dim DDKF As New DiscreteDoubleKeyFrame
        DDKF.Value = Me.ActualWidth
        DDKF.KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromSeconds(ImageStillDisplaySec))
        myDoubleAnimationX.KeyFrames.Add(DDKF)


        Dim linearFadeIn As New LinearDoubleKeyFrame
        linearFadeIn.Value = 1
        linearFadeIn.KeyTime = KeyTime.FromTimeSpan(TimeSpan.FromSeconds(ImageFadeInSec))
        imageExitAnimation.KeyFrames.Add(linearFadeIn)




        'scaleAnimatiionX.From = 1
        'scaleAnimatiionX.To = 2



        'scaleAnimatiionY.From = 1
        'scaleAnimatiionY.To = 2




        scaleTrans.CenterX = ImageCentreX '  imageBoxSize.Width / 2 ' X ' 1024 / 2 'X 'image.ActualWidth / 2
        scaleTrans.CenterY = ImageCentreY  'imageBoxSize.Height / 2 'Y ' 1920 / 2 'Y 'image.ActualHeight / 2






        panX.EasingFunction = easingFunction
        panY.EasingFunction = easingFunction


        'panX.BeginTime = TimeSpan.FromSeconds(fadeOutSec + 3)
        'panY.BeginTime = TimeSpan.FromSeconds(fadeOutSec + 3)


        panX.Duration = New Duration(TimeSpan.FromSeconds(PanZoomDurationSec))
        panY.Duration = New Duration(TimeSpan.FromSeconds(PanZoomDurationSec))


        Dim centreScreen As New Point(Me.ActualWidth / 2, Me.ActualHeight / 2)


        panX.To = (ImageCentreX - faceCentreX) * 2
        panY.To = (ImageCentreY - faceCentreY) * 2


        Dim transGroup As New TransformGroup
        transGroup.Children.Add(scaleTrans)
        transGroup.Children.Add(panTrans)

        'Debug.WriteLine(Me.ActualWidth & vbTab & vbTab & Me.ActualHeight & vbTab & centreX & vbTab & centreY & vbTab & imageBoxSize.Width & vbTab & imageBoxSize.Height & vbTab & scaleTrans.CenterX & vbTab & scaleTrans.CenterY & vbTab & System.IO.Path.GetFileNameWithoutExtension(filename))




        'image.RenderTransform = scaleTrans
        image.RenderTransform = transGroup
        'image.RenderTransformOrigin = New Point(0.5, 0.5)


        entryStoryBoard.Children.Add(imageLoadAnimation)
        entryStoryBoard.Children.Add(scaleAnimatiionX)
        entryStoryBoard.Children.Add(scaleAnimatiionY)
        entryStoryBoard.Children.Add(panX)
        entryStoryBoard.Children.Add(panY)


        entryStoryBoard.Children.Add(myDoubleAnimationX)

        exitStoryBoard.Children.Add(imageExitAnimation)




        Storyboard.SetTargetProperty(imageLoadAnimation, "Opacity")
        Storyboard.SetTargetProperty(scaleAnimatiionX, "ScaleX")
        Storyboard.SetTargetProperty(scaleAnimatiionY, "ScaleY")
        Storyboard.SetTargetProperty(panX, "X")
        Storyboard.SetTargetProperty(panY, "Y")

        Storyboard.SetTargetProperty(myDoubleAnimationX, "ActualWidth")

        Storyboard.SetTargetProperty(imageExitAnimation, "Opacity")






        Storyboard.SetTarget(scaleAnimatiionX, scaleTrans)
        Storyboard.SetTarget(scaleAnimatiionY, scaleTrans)
        Storyboard.SetTarget(myDoubleAnimationX, Me)
        Storyboard.SetTarget(imageExitAnimation, overlayRect)

        Storyboard.SetTarget(imageLoadAnimation, image)
        Storyboard.SetTarget(panX, panTrans)
        Storyboard.SetTarget(panY, panTrans)




        Debug.WriteLine("Start : " & DateTime.Now)
        entryStoryBoard.Begin()

        processNext = False

        While processNext = False
            Await Task.Delay(100)
        End While
    End Function
    Private Sub exitStoryBoard_Completed(sender As Object, e As Object) Handles exitStoryBoard.Completed
        processNext = True
        overlayRect.Opacity = 1
        Debug.WriteLine("Complete : " & DateTime.Now)
        exitStoryBoard.Stop()
    End Sub

    Private Sub entryStoryBoard_Completed(sender As Object, e As Object) Handles entryStoryBoard.Completed
        panTrans.X = panX.To
        panTrans.Y = panY.To
        scaleTrans.ScaleX = scaleAnimatiionX.To
        scaleTrans.ScaleY = scaleAnimatiionY.To
        entryStoryBoard.Stop()
        exitStoryBoard.Begin()
    End Sub
End Class
