//
//  SettingsController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit

class SettingsController: UIViewController {

    @IBOutlet weak var stepsInput: UITextField!
    @IBOutlet weak var speedInput: UITextField!
    @IBOutlet weak var acceleration: UITextField!
    
    @IBOutlet weak var retryButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        view.addGestureRecognizer(UITapGestureRecognizer(target: self, action: #selector(dismissKeyboard)))
    }
    
    @objc func dismissKeyboard() {
        view.endEditing(true)
    }
    

    @IBAction func onRetry(_ sender: Any) {
    }
    
    
    
}
