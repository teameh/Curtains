//
//  RequestController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit

class RequestController: UIViewController {

    @IBOutlet weak var retryButton: UIButton!
    @IBOutlet weak var statusLabel: UILabel!

    override func viewDidLoad() {
        super.viewDidLoad()

        self.statusLabel.text = "";
        self.retryButton.alpha = 0
        self.retryButton.isEnabled = false
    }

    func onLoading(status: String) {
        self.statusLabel.text = status;
        self.retryButton.alpha = 0
        self.retryButton.removeTarget(nil, action: nil, for: .allEvents)
    }

    func onSuccess(status: String) {
        self.statusLabel.text = status;
        self.retryButton.alpha = 0
        self.retryButton.isEnabled = false

        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(3)) {
            self.statusLabel.text = ""
        }
    }

    func onFailure(status: String, action: Selector) {
        self.retryButton.alpha = 1
        self.statusLabel.text = status;
        self.retryButton.isEnabled = true
        self.retryButton.removeTarget(nil, action: nil, for: .allEvents)
        self.retryButton.addTarget(self, action: action, for: .touchUpInside)
    }

     

}
